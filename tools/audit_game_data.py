#!/usr/bin/env python3
"""Inspect local phase-00 inputs without extracting or modifying game assets.

Write output under build/ (ignored). HDT offsets describe allocated spans, not
proven payload lengths. Neither unreferenced bytes nor trailing bytes are called
padding without format-specific evidence. This is an audit tool, not a runtime.
"""
import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path

from extract_baserom import EXPECT_SHA1, EXPECT_SHA256

OFFSET_COUNT = 33  # afile_init copies 0x20 + 1 words


def validate_hdt(data, bin_size):
    if not 0 <= bin_size <= 0xFFFFFFFF:
        raise ValueError('BIN size is outside the 32-bit archive address space')
    if len(data) < OFFSET_COUNT * 4:
        raise ValueError('HDT is shorter than the 33 offsets consumed by afile_init')
    offsets = struct.unpack_from('<33I', data)
    if any(a > b for a, b in zip(offsets, offsets[1:])):
        raise ValueError('HDT offsets decrease')
    if offsets[-1] > bin_size:
        raise ValueError('HDT end sentinel exceeds BIN size')
    return {
        'offset_count_consumed': OFFSET_COUNT,
        'member_count_from_loader': OFFSET_COUNT - 1,
        'hdt_extra_bytes_unclassified': len(data) - OFFSET_COUNT * 4,
        'hdt_extra_bytes_all_zero': not any(data[OFFSET_COUNT * 4:]),
        'bin_prefix_bytes_unclassified': offsets[0],
        'bin_tail_bytes_unclassified': bin_size - offsets[-1],
        'end_sentinel_equals_bin_size': offsets[-1] == bin_size,
        'members': [{'index': i, 'offset': start, 'end': end, 'span_bytes': end-start,
                     'start_sector': start // 2048, 'sector_skip': start % 2048,
                     'read_sector_count': (end+2047)//2048 - start//2048,
                     'payload_bytes': None, 'padding_bytes': None}
                    for i, (start, end) in enumerate(zip(offsets, offsets[1:]))],
    }


class ExeView:
    def __init__(self, data):
        if len(data) < 0x800 or data[:8] != b'PS-X EXE':
            raise ValueError('Missing/truncated PS-X EXE header')
        self.data = data
        self.load, self.size = struct.unpack_from('<II', data, 0x18)
        if self.size > len(data) - 0x800:
            raise ValueError('EXE load size exceeds file')

    def read(self, address, size):
        if size < 0 or not self.load <= address <= self.load + self.size - size:
            raise ValueError(f'EXE pointer/range outside loaded image: 0x{address:X}')
        offset = 0x800 + address - self.load
        return self.data[offset:offset+size]

    def word(self, address):
        return struct.unpack('<I', self.read(address, 4))[0]

    def string(self, address, limit=256):
        length = min(limit, self.load + self.size - address)
        raw = self.read(address, length)
        if b'\0' not in raw:
            raise ValueError('Unterminated executable string')
        return raw.split(b'\0', 1)[0].decode('ascii')


def inspect_exe(data):
    if hashlib.sha1(data).hexdigest() != EXPECT_SHA1 or hashlib.sha256(data).hexdigest() != EXPECT_SHA256:
        raise ValueError('EXE hashes do not match pristine US SLUS_011.15; fixed table addresses are unsafe')
    view = ExeView(data)
    records = []
    for i in range(32):
        record = view.read(0x80048B1C + i * 0x28, 0x28)
        records.append({'index': i, 'flags_raw': f'0x{struct.unpack_from("<I", record)[0]:08X}',
                        'name_bytes_hex': record[4:0x24].hex(),
                        'offset_before_hdt_init': struct.unpack_from('<I', record, 0x24)[0]})
    xa = [{'index': i, 'pointer': f'0x{view.word(0x80049A70+i*4):08X}',
           'disc_path': view.string(view.word(0x80049A70+i*4))} for i in range(4)]
    return {'load_address': f'0x{view.load:08X}', 'load_size': view.size,
            'hdt_disc_path': view.string(0x80010078, 16),
            'bin_disc_path': view.string(0x80010088, 16),
            'overlay_region_pointers': [f'0x{view.word(0x80010098+i*4):08X}' for i in range(2)],
            'xa_files': xa, 'afile_records_before_init': records,
            'sound_offsets_first_three': [view.word(0x80049370+i*4) for i in range(3)],
            'sound_offset_count': None,
            'cd_audio_tracks_address': '0x80049A80', 'cd_audio_track_count': None}


def hash_range(stream, start, size):
    if start < 0 or size < 0:
        raise ValueError('Negative BIN range')
    stream.seek(start)
    digest = hashlib.sha256()
    while size:
        block = stream.read(min(size, 1024 * 1024))
        if not block:
            raise ValueError('Unexpected EOF while hashing BIN span')
        digest.update(block)
        size -= len(block)
    return digest.hexdigest()


def file_identity(path):
    size = path.stat().st_size
    with path.open('rb') as stream:
        return {'file': path.name, 'size': size,
                'sha256': hash_range(stream, 0, size)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--hdt', type=Path)
    parser.add_argument('--bin', type=Path)
    parser.add_argument('--exe', type=Path)
    parser.add_argument('--output', type=Path, help='Local report; use an ignored path such as build/audit/data.json')
    args = parser.parse_args()
    if bool(args.hdt) != bool(args.bin) or not (args.hdt or args.exe):
        parser.error('provide --hdt and --bin together, and/or --exe')
    try:
        report = {'format': 'btn-phase00-data-audit-v1', 'inputs': {},
                  'scope': 'structural spans and executable metadata; no payload format validation'}
        if args.hdt:
            report['inputs']['hdt'] = file_identity(args.hdt)
            report['inputs']['bin'] = file_identity(args.bin)
            report['archive'] = validate_hdt(args.hdt.read_bytes(), args.bin.stat().st_size)
            with args.bin.open('rb') as stream:
                for member in report['archive']['members']:
                    member['span_sha256'] = hash_range(stream, member['offset'], member['span_bytes'])
        if args.exe:
            report['inputs']['exe'] = file_identity(args.exe)
            report['executable'] = inspect_exe(args.exe.read_bytes())
        text = json.dumps(report, indent=2) + '\n'
        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(text)
        else:
            print(text, end='')
        return 0
    except (OSError, ValueError, struct.error, UnicodeError) as error:
        print(f'audit failed: {error}', file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
