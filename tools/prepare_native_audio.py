#!/usr/bin/env python3
"""Export the verified US sound table to local, format-bounded VAB/SEQ files.

Run only against a user-supplied game; output belongs under ignored build/.
Source contracts: sound_ui.c:sound_load_step and cd_audio.c:cd_audio_play_track.
No XA sectors are converted to 2048-byte data sectors by this tool.
"""
import argparse
import json
import struct
import sys
from pathlib import Path

from audit_game_data import ExeView, file_identity, inspect_exe, validate_hdt


def bank_layout(header, body_size):
    if len(header) < 32 or header[:4] != b'pBAV':
        raise ValueError('Missing VAB header')
    version, = struct.unpack_from('<I', header, 4)
    programs, tones, samples = struct.unpack_from('<3H', header, 18)
    if version != 7 or not 1 <= programs <= 128 or not 1 <= samples <= 255:
        raise ValueError('Unsupported VAB version/counts')
    size_table = 0x820 + programs * 512
    header_size = size_table + 512
    if len(header) < header_size:
        raise ValueError('Truncated VAB tone/size table')
    sizes = struct.unpack_from('<256H', header, size_table)
    if sizes[0] or any(sizes[samples + 1:]):
        raise ValueError('Unexpected reserved VAB sample sizes')
    lengths = [n * 8 for n in sizes[1:samples + 1]]
    if any(n < 16 or n % 16 for n in lengths) or sum(lengths) > body_size:
        raise ValueError('VAB samples exceed body or have invalid block lengths')
    return dict(programs=programs, tones=tones, samples=samples,
                header_bytes=header_size, sample_bytes=lengths,
                body_bytes=sum(lengths), body_tail_bytes=body_size-sum(lengths))


def read_catalog(data_dir):
    exe = (data_dir / 'SLUS_011.15').read_bytes()
    metadata = inspect_exe(exe)  # Pristine hash gate before any fixed address.
    view = ExeView(exe)
    archive = data_dir / 'A_FILE.BIN'
    hdt = validate_hdt((data_dir / 'A_FILE.HDT').read_bytes(), archive.stat().st_size)
    member = hdt['members'][9]
    # soundOffsets ends at soundDefs; pairs address [start, body-or-1, end].
    offsets = struct.unpack('<151I', view.read(0x80049370, 0x800495CC-0x80049370))
    if offsets[0] or offsets[-1] != member['span_bytes']:
        raise ValueError('Sound table does not cover member 9')
    resources = []
    with archive.open('rb') as stream:
        for index in range(75):
            start, middle, end = offsets[index*2:index*2+3]
            if not 0 <= start < end <= member['span_bytes']:
                raise ValueError(f'Invalid sound span {index}')
            stream.seek(member['offset'] + start)
            data = stream.read(end-start)
            if len(data) != end-start:
                raise ValueError('Short sound read')
            record = dict(index=index, offset=start, end=end, span_bytes=end-start)
            if middle == 1:
                if data[:8] != b'pQES\0\0\0\1':
                    raise ValueError(f'Sequence {index} does not match its table marker')
                record.update(kind='sequence', file=f'sequence-{index:03}.seq')
                payload = data
            else:
                if not start < middle < end:
                    raise ValueError(f'Invalid VAB body offset {index}')
                layout = bank_layout(data[:middle-start], end-middle)
                record.update(kind='bank', file=f'bank-{index:03}.vab',
                              body_offset=middle, **layout)
                # Join logical VH/VB; retain padding extents in metadata only.
                payload = data[:layout['header_bytes']] + data[middle-start:middle-start+layout['body_bytes']]
            resources.append((record, payload))
    definitions = [list(struct.unpack('<7H', view.read(0x800495CC+i*14, 14))) for i in range(78)]
    tracks = [dict(index=i, fade_ticks=a, file=b, channel=c)
              for i in range(26)
              for a, b, c in [struct.unpack('<3H', view.read(0x80049A80+i*6, 6))]]
    if any(t['file'] > 3 or t['channel'] > 31 for t in tracks):
        raise ValueError('Invalid XA track table')
    xa = []
    for entry in metadata['xa_files']:
        parts = entry['disc_path'].lstrip('\\').replace('\\', '/').split('/')
        parts[-1] = parts[-1].split(';')[0]
        local = data_dir.joinpath(*parts)
        xa.append(dict(**entry, available=local.is_file(),
                       local_relative_path='/'.join(parts)))
    catalog = dict(format='BTN native audio inventory 1',
                   inputs=[file_identity(data_dir/name) for name in ('SLUS_011.15','A_FILE.HDT','A_FILE.BIN')],
                   sound_table_address='0x80049370', member=9,
                   resources=[r for r, _ in resources], xa_files=xa, xa_tracks=tracks,
                   sound_definitions=dict(address='0x800495CC',
                       fields=['vab_slot','category','program','tone','note','fine','volume'], rows=definitions),
                   caveats=['Bank/sequence pairing must be selected from scene load requests; adjacency is not proof.',
                            'XA file presence does not verify sector layout or playback.',
                            'PCM export and synthetic tests do not establish PS1 audio parity.'])
    return catalog, resources


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--data-dir', type=Path, required=True)
    parser.add_argument('--output-dir', type=Path, required=True)
    args = parser.parse_args(argv)
    try:
        catalog, resources = read_catalog(args.data_dir)
        args.output_dir.mkdir(parents=True, exist_ok=False)
        for record, payload in resources:
            with (args.output_dir/record['file']).open('xb') as stream:
                stream.write(payload)
        with (args.output_dir/'audio-inventory.json').open('x', encoding='utf-8') as stream:
            json.dump(catalog, stream, indent=2)
            stream.write('\n')
    except (OSError, ValueError, struct.error) as error:
        print(f'Audio export failed: {error}', file=sys.stderr)
        return 1
    print(f'Exported {len(resources)} sound resources; XA available: {sum(x["available"] for x in catalog["xa_files"])}/4')
    return 0


if __name__ == '__main__':
    sys.exit(main())
