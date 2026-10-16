"""Extract the editable text bank from a baserom, or compile it into assembly.

The game's text is not distributed with this repository. `extract` decodes it
from your own baserom/SLUS_011.15 into src/text/text_bank.json, following the
named entries in src/text/text_bank.layout.json. `build` (the default when
only --output is given) compiles that JSON back into the data assembly.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
from pathlib import Path
from text_codec import decode, encode

BANK_BASE = 0x8005175C
BANK_SIZE = 13406
EXE_LOAD = 0x80010000 - 0x800  # PS-X EXE header precedes the image
PAD_SUFFIX = re.compile(r'\{PAD:\d+\}$')
DATA_WORD = re.compile(r'(\/\* [0-9A-Fa-f]+ ([0-9A-Fa-f]{8}) )([0-9A-Fa-f]{8})( \*\/ \.word )0x[0-9A-Fa-f]{8}')
DATA_SHORT = re.compile(r'(\/\* [0-9A-Fa-f]+ ([0-9A-Fa-f]{8}) \*\/ \.short )0x[0-9A-Fa-f]{4}')


def encode_bank(document: dict) -> bytes:
    if document.get('format') != 'btnx-text-bank-v1' or int(document['baseAddress'], 16) != BANK_BASE or document['byteSize'] != BANK_SIZE:
        raise ValueError('Unexpected text-bank format or retail layout')
    payload = bytearray()
    names = set()
    for entry in document['entries']:
        if entry['offset'] != len(payload) or entry['name'] in names:
            raise ValueError('Missing, overlapping, reordered or duplicate entry: ' + entry['name'])
        names.add(entry['name'])
        capacity = entry['capacity']
        if not isinstance(capacity, int) or capacity <= 0 or capacity * 2 > BANK_SIZE - len(payload):
            raise ValueError('Invalid capacity: ' + entry['name'])
        words = encode(entry['text'])
        if len(words) > capacity:
            raise ValueError(f"{entry['name']} needs {len(words)} glyph words; capacity is {capacity}")
        if 0xFFFF not in words:
            raise ValueError('Text entry needs an explicit {END}: ' + entry['name'])
        words += [0] * (capacity - len(words))
        payload.extend(struct.pack('<' + 'H' * capacity, *words))
    if len(payload) != BANK_SIZE:
        raise ValueError(f'Incomplete bank: {len(payload)} bytes, expected {BANK_SIZE}')
    return bytes(payload)


def extract_bank(layout: dict, rom: bytes) -> dict:
    """Decode the retail bank bytes into a text document using a text-free layout."""
    if layout.get('format') != 'btnx-text-bank-v1' or int(layout['baseAddress'], 16) != BANK_BASE or layout['byteSize'] != BANK_SIZE:
        raise ValueError('Unexpected text-bank layout')
    start = BANK_BASE - EXE_LOAD
    payload = rom[start:start + BANK_SIZE]
    if len(payload) != BANK_SIZE or hashlib.sha256(payload).hexdigest() != layout['originalSha256']:
        raise ValueError('baserom text bank does not match the US retail SLUS_011.15')
    entries = []
    for entry in layout['entries']:
        offset, capacity = entry['offset'], entry['capacity']
        words = list(struct.unpack_from('<' + 'H' * capacity, payload, offset))
        # encode_bank pads every entry back to its capacity, so trailing padding is implicit.
        text = PAD_SUFFIX.sub('', decode(words))
        entries.append({'name': entry['name'], 'offset': offset, 'capacity': capacity, 'text': text})
    document = {key: value for key, value in layout.items() if key != 'entries'}
    document['entries'] = entries
    if encode_bank(document) != payload:
        raise ValueError('Extracted text bank does not round-trip to the retail bytes')
    return document


def load_bank(path: Path) -> bytes:
    return encode_bank(json.loads(path.read_text(encoding='utf-8')))


def replace_bank(assembly: str, payload: bytes) -> str:
    if len(payload) != BANK_SIZE:
        raise ValueError('Unexpected bank size')
    replaced = 0
    def replace(match):
        nonlocal replaced
        address = int(match.group(2), 16)
        if not BANK_BASE <= address < BANK_BASE + BANK_SIZE:
            return match.group()
        offset = address - BANK_BASE
        original = bytes.fromhex(match.group(3))
        width = min(4, BANK_SIZE - offset)
        data = payload[offset:offset + width] + original[width:]
        replaced += width
        return match.group(1) + data.hex().upper() + match.group(4) + f'0x{int.from_bytes(data, "little"):08X}'
    result = DATA_WORD.sub(replace, assembly)
    def replace_short(match):
        nonlocal replaced
        address = int(match.group(2), 16)
        if not BANK_BASE <= address < BANK_BASE + BANK_SIZE:
            return match.group()
        offset = address - BANK_BASE
        replaced += 2
        return match.group(1) + f'0x{int.from_bytes(payload[offset:offset + 2], "little"):04X}'
    result = DATA_SHORT.sub(replace_short, result)
    if replaced != BANK_SIZE:
        raise ValueError(f'Bank assembly coverage is {replaced}, expected {BANK_SIZE}')
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', nargs='?', choices=('build', 'extract'), default='build')
    parser.add_argument('--bank', type=Path, default=Path('src/text/text_bank.json'))
    parser.add_argument('--layout', type=Path, default=Path('src/text/text_bank.layout.json'))
    parser.add_argument('--baserom', type=Path, default=Path('baserom/SLUS_011.15'))
    parser.add_argument('--assembly', type=Path, default=Path('asm/data/39314.data.s'))
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    if args.command == 'extract':
        layout = json.loads(args.layout.read_text(encoding='utf-8'))
        document = extract_bank(layout, args.baserom.read_bytes())
        output = args.output or args.bank
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(document, indent=2, ensure_ascii=False) + '\n', encoding='utf-8', newline='\n')
        return 0
    if args.output is None:
        parser.error('build requires --output')
    generated = replace_bank(args.assembly.read_text(), load_bank(args.bank))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(generated, encoding='utf-8', newline='\n')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
