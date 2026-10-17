#!/usr/bin/env python3
"""Prepare local phase-02 metadata, text bytes, and a bounded resource inventory.

Requires the hash-verified US executable alongside A_FILE.HDT/BIN. No retail
bytes are embedded here. Output must be a new directory; use ignored build/.
Container probes inspect structure only, never execute overlays or decode pixels.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys

from audit_game_data import ExeView, file_identity, inspect_exe, validate_hdt
from extract_baserom import EXE_LEN
from text_bank import BANK_BASE, BANK_SIZE, extract_bank

ROOT = Path(__file__).resolve().parents[1]


class MemberView:
    def __init__(self, stream, start, size):
        if start < 0 or size < 0:
            raise ValueError('Negative member range')
        self.stream, self.start, self.size = stream, start, size

    def read(self, offset, size):
        if offset < 0 or size < 0 or offset > self.size or size > self.size - offset:
            raise ValueError('Resource structure exceeds its member/slot span')
        self.stream.seek(self.start + offset)
        data = self.stream.read(size)
        if len(data) != size:
            raise ValueError('BIN shortened while inspecting resource')
        return data


def inspect_container_0d(view):
    header = view.read(0, 4)
    if header[0] != 13:
        raise ValueError('Missing container 0D signature')
    count = struct.unpack_from('<H', header, 2)[0]
    if count > (view.size - 4) // 12:
        raise ValueError('Container 0D block count exceeds span')
    blocks, cursor = [], 4
    for _ in range(count):
        word, x, y, width, height = struct.unpack('<I4H', view.read(cursor, 12))
        length, flags = word & 0xFFFFFF, word >> 24
        aligned = (length + 3) & ~3
        if aligned > view.size - cursor - 12:
            raise ValueError('Container 0D payload exceeds span')
        blocks.append({'header_offset': cursor, 'payload_bytes': length, 'flags_raw': flags,
                       'vram_rectangle_words': [x, y, width, height]})
        cursor += 12 + aligned
    return {'block_count': count, 'byte_size': cursor, 'blocks': blocks,
            'validation': 'Headers and declared block extents only; compressed pixel streams not decoded',
            'source': 'src/graphics/archive_3.c:DecodeContainer0D'}


def build_manifest(archive, executable, inputs):
    spans, records = archive['members'], executable['afile_records_before_init']
    if len(spans) != 32 or len(records) != 32:
        raise ValueError('Expected exactly 32 member spans and executable records')
    members = []
    for index, (span, record) in enumerate(zip(spans, records)):
        if span['index'] != index or record['index'] != index:
            raise ValueError('Member indices do not match executable record order')
        raw = bytes.fromhex(record['name_bytes_hex'])
        if len(raw) != 32:
            raise ValueError('Executable member name field must contain 32 bytes')
        try:
            name = raw.split(b'\0', 1)[0].decode('ascii')
        except UnicodeError:
            name = ''
        flags = int(record['flags_raw'], 16)
        if not 0 <= flags <= 0xFFFFFFFF:
            raise ValueError('Member flags exceed unsigned LE32')
        members.append({**span, 'name': name or str(index), 'name_bytes_hex': raw.hex(),
                        'flags_raw': f'0x{flags:08X}'})
    return {'format': 'btn-native-member-manifest-v1', 'inputs': inputs,
            'flags_scope': 'Raw executable values; no inferred flag semantics', 'members': members}


def inventory_member(stream, member):
    view = MemberView(stream, member['offset'], member['span_bytes'])
    head = view.read(0, min(36, view.size))
    entry = {'index': member['index'], 'name': member['name'], 'head_hex': head[:16].hex(),
             'head_format': 'unknown', 'span_bytes': view.size}
    # An overlay's leading ID can also be 0D/0E. Use verified names before magic.
    if member['name'].lower().startswith('obj\\') and member['flags_raw'] == '0x00000000':
        entry.update(head_format='ps1-overlay-by-executable-metadata',
                     validation='Role from executable path and main scene calls; not a machine-code decoder')
        return entry
    if head[:1] == b'\x0D':
        try:
            entry.update(head_format='container0d-prefix', container=inspect_container_0d(view))
            entry['remaining_span_bytes_unclassified'] = view.size - entry['container']['byte_size']
        except ValueError as error:
            entry.update(head_format='invalid-container0d-candidate', error=str(error))
    elif len(head) >= 36 and struct.unpack_from('<I', head)[0] == 14:
        offsets = list(struct.unpack_from('<5I', head, 16))
        entry.update(head_format='animation0e-header', relative_table_offsets=offsets,
                     table_offsets_within_span=all(offset <= view.size for offset in offsets),
                     validation='Header/table starts only; table lengths and animation payloads not decoded',
                     source='src/graphics/anim.c:anim_bind_file')
    elif head[:4] == b'pBAV':
        entry.update(head_format='vab-header-prefix',
                     validation='Signature only; sequence/body boundaries require sound table audit',
                     source='src/sound/sound_ui.c:sound_load_step')
    elif head[:4] == b'\x10\0\0\0':
        entry.update(head_format='tim-signature-candidate', validation='Signature only; no TIM decode')

    if member['name'].lower().split('\\')[-1] in {f'messege{i}.bin' for i in range(5)}:
        counts = {'slots': view.size // 8192, 'unused_marker': 0, 'valid_container0d': 0,
                  'invalid_container0d': 0, 'unknown': 0, 'trailing_bytes': view.size % 8192}
        first_sample = None
        for slot in range(counts['slots']):
            prefix = view.read(slot * 8192, 8)
            if prefix[:4] == b'\xFF' * 4:
                counts['unused_marker'] += 1
            elif prefix[4] == 13:
                try:
                    container = inspect_container_0d(MemberView(stream, view.start + slot * 8192 + 4, 8188))
                    counts['valid_container0d'] += 1
                    if first_sample is None:
                        first_sample = {'slot': slot, 'member_offset': slot * 8192 + 4, **container}
                except ValueError:
                    counts['invalid_container0d'] += 1
            else:
                counts['unknown'] += 1
        entry.update(message_slots=counts, first_container_sample=first_sample,
                     slot_layout_source='src/ui/menu_window.c:menu_begin_load; src/ui/ui.c:DecodeMessageSlotContainer')
        if counts['valid_container0d'] and not any(counts[key] for key in
                ('invalid_container0d', 'unknown', 'trailing_bytes')):
            entry['head_format'] = 'message-slot-array0d'
    return entry


def prepare(data_dir, output_dir):
    if output_dir.exists():
        raise ValueError('Output directory already exists; refusing to overwrite')
    hdt_path, bin_path, exe_path = (data_dir / name for name in ('A_FILE.HDT', 'A_FILE.BIN', 'SLUS_011.15'))
    if exe_path.stat().st_size != EXE_LEN:
        raise ValueError('EXE hashes cannot match the US executable: unexpected file size')
    raw_exe = exe_path.read_bytes()
    executable = inspect_exe(raw_exe)  # Fixed addresses are unsafe before identity checks.
    with hdt_path.open('rb') as stream:
        archive = validate_hdt(stream.read(33 * 4), bin_path.stat().st_size)
    inputs = {name: file_identity(path) for name, path in
              (('hdt', hdt_path), ('bin', bin_path), ('exe', exe_path))}
    manifest = build_manifest(archive, executable, inputs)
    layout = json.loads((ROOT / 'src/text/text_bank.layout.json').read_text(encoding='utf-8'))
    text = extract_bank(layout, raw_exe)
    text_bytes = ExeView(raw_exe).read(BANK_BASE, BANK_SIZE)
    with bin_path.open('rb') as stream:
        inventory = [inventory_member(stream, member) for member in manifest['members']]
    metadata = {'format': 'btn-native-executable-data-v1', 'input': inputs['exe'],
                'executable': executable,
                'text_bank': {'file': 'text-bank.bin', 'address_space': 'ps1-virtual',
                              'ps1_base_address': f'0x{BANK_BASE:08X}', 'byte_size': BANK_SIZE,
                              'sha256': hashlib.sha256(text_bytes).hexdigest()},
                'address_policy': 'PS1 addresses are serialized metadata, never host pointers',
                'remaining_data_requirements': [
                    'External XA files listed in executable.xa_files are not supplied by this tool',
                    'Sound offset/track counts remain unverified; delimit them before phase-04 decoding',
                    'Additional scene tables are selected with their phase-03/05 consumers, not guessed']}
    output_dir.mkdir(parents=True)
    for name, document in (('member-manifest.json', manifest), ('executable-data.json', metadata),
                           ('text-bank.json', text), ('resource-inventory.json', {
                               'format': 'btn-native-resource-inventory-v1', 'inputs': inputs,
                               'scope': 'Bounded structural probes; unknown payloads explicitly retained',
                               'members': inventory})):
        (output_dir / name).write_text(json.dumps(document, indent=2) + '\n', encoding='utf-8')
    (output_dir / 'text-bank.bin').write_bytes(text_bytes)
    return manifest, text, inventory


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--data-dir', required=True, type=Path)
    parser.add_argument('--output-dir', required=True, type=Path)
    args = parser.parse_args()
    try:
        manifest, text, inventory = prepare(args.data_dir, args.output_dir)
        unknown = sum(member['head_format'] == 'unknown' for member in inventory)
        print(f"native-data: {len(manifest['members'])} verified name/flag records, "
              f"{len(text['entries'])} round-trip-verified text entries, "
              f"{len(inventory)} inventory records ({unknown} unknown head formats)")
        return 0
    except (OSError, ValueError, KeyError, struct.error) as error:
        print(f'native-data failed: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
