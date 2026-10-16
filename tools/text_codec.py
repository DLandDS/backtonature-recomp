"""Lossless BTN glyph codec for editable TEXT("...") C initializers.

The glyph atlas and apostrophe mapping follow the BTNx translation project's
verified font atlas.
Runtime commands follow our TextControlCode declarations and the insert-code
resolver in include/game_types.h and src/text/dialog.c.
No locale translation or automatic terminator insertion happens here.
"""
from __future__ import annotations

import argparse
import ast
import re
from pathlib import Path

CHAR_TO_CODE = {}
for row, chars in enumerate(('ABCDEFGHIJKLMNOP', 'QRSTUVWXYZ', 'abcdefghijklmnop', 'qrstuvwxyz', '0123456789')):
    CHAR_TO_CODE.update({char: (row << 4) | col for col, char in enumerate(chars)})
CHAR_TO_CODE.update({"'t": 0x50, "'s": 0x51, "'l": 0x52, "'d": 0x53, "'m": 0x54, "'r": 0x55, "'v": 0x56,
                     '"': 0x8E, '?': 0x62, '!': 0x63, ':': 0x66, ';': 0x67, '+': 0x68, '-': 0x69,
                     '(': 0x6A, ')': 0x6B, '.': 0x6C, '_': 0x74, '~': 0x75, '&': 0x76,
                     ',': 0x8C, "'": 0x8D, '%': 0x8F, ' ': 0xFFD, '\n': 0xFFFE})
TOKEN_TO_CODE = {
    'QUOTE_LEGACY': 0x61, 'APOSTROPHE_LEGACY': 0x65, 'APOSTROPHE': 0x8D,
    'DASH_ALT': 0x60, 'CIRCLE': 0x6D, 'CROSS': 0x6E, 'TRIANGLE': 0x6F, 'SQUARE': 0x70,
    'SLASH': 0x71, 'DOWN': 0x72, 'UP': 0x73, 'STAR': 0x77, 'STAR_ALT': 0x78,
    'DIGIT_1_ALT': 0x79, 'DIGIT_2_ALT': 0x7A, 'DIGIT_3_ALT': 0x7B,
    'DIGIT_4_ALT': 0x7C, 'DIGIT_5_ALT': 0x7D, 'DIGIT_6_ALT': 0x7E,
    'ELLIPSIS': 0x7F, 'HEART': 0x80, 'HEART_ALT': 0x81,
    'END': 0xFFFF, 'PAGE_MARK': 0xFFC, 'PAGE_END': 0xFFB,
    'CHOICE_2': 0xFFA, 'CHOICE_3': 0xFF9, 'CHOICE_4': 0xFF8,
    'FACE_BANK': 0xFF7, 'FACE_HIDE': 0xFF6, 'FACE_RELOAD': 0xFF5, 'FACE_SHOW': 0xFF4,
    'NO_SCROLL': 0xFF3, 'FLAG_FF2': 0xFF2, 'SCROLL_HOLD': 0xFF1,
}
INSERT_NAMES = ('PLAYER_NAME', 'HORSE_NAME', 'DOG_NAME', 'FARM_NAME', 'CHILD_NAME', 'FAMILY_ADDRESS',
                'STAGED_VALUE', 'BASKET_COUNT', 'FODDER_COUNT', 'LUMBER_COUNT', 'CHICKEN_FEED_COUNT',
                'FISH_FOOD_COUNT', 'SLOT_0_LIVESTOCK_NAME', 'SLOT_1_LIVESTOCK_NAME', 'MINIGAME_TEXT',
                'EVENT_LIVESTOCK_NAME', 'EVENT_LIVESTOCK_NAME_ALT', 'AGED_LIVESTOCK_LIST',
                'AGED_FLAGGED_LIVESTOCK_LIST', 'AGED_CHICKEN_LIST', 'LIVESTOCK_LIST',
                'FLAGGED_LIVESTOCK_LIST', 'CHICKEN_LIST', 'ANN_BIRTHDAY_DAY', 'HELD_CHICKEN_NAME')
TOKEN_TO_CODE.update({name: 0x4000 | (index + 10) for index, name in enumerate(INSERT_NAMES)})
CODE_TO_CHAR = {code: char for char, code in CHAR_TO_CODE.items()}
CODE_TO_TOKEN = {code: name for name, code in TOKEN_TO_CODE.items()}
LITERAL = r'"(?:\\.|[^"\\])*"'
SOURCE_TOKEN = re.compile(r'/\*.*?\*/|//[^\n]*|\x27(?:\\.|[^\x27\\])*\x27|' + LITERAL + r'|\bTEXT\s*\(((?:\s*' + LITERAL + r')+)\s*\)', re.S)


def encode(text: str) -> list[int]:
    words = []
    index = 0
    while index < len(text):
        if text[index] == '{':
            end = text.find('}', index)
            if end < 0:
                raise ValueError('Unclosed text token')
            token = text[index + 1:end]
            if token in TOKEN_TO_CODE:
                words.append(TOKEN_TO_CODE[token])
            elif re.fullmatch(r'RAW:[0-9A-Fa-f]{1,4}', token):
                words.append(int(token[4:], 16))
            elif re.fullmatch(r'PAD:[0-9]{1,5}', token) and int(token[4:]) <= 65535:
                words.extend([0] * int(token[4:]))
            elif re.fullmatch(r'FACE:[0-9A-Fa-f]{3,4}', token) and 0xE38 <= int(token[5:], 16) <= 0xFEF:
                words.append(int(token[5:], 16))
            elif re.fullmatch(r'WAIT:[0-9]{1,2}', token) and int(token[5:]) <= 10:
                words.append(0xE2F - int(token[5:]))
            else:
                raise ValueError('Unknown or invalid text token: {' + token + '}')
            index = end + 1
            continue
        glyph = text[index:index + 2]
        if glyph not in CHAR_TO_CODE:
            glyph = text[index]
        if glyph not in CHAR_TO_CODE:
            raise ValueError(f'Unsupported glyph {glyph!r} at character {index}')
        words.append(CHAR_TO_CODE[glyph])
        index += len(glyph)
    return words


def decode(words: list[int]) -> str:
    parts = []
    index = 0
    while index < len(words):
        word = words[index]
        if not 0 <= word <= 65535:
            raise ValueError(f'Halfword out of range: {word}')
        if word == 0 and index > 0 and words[index - 1] == 0xFFFF:
            end = index
            while end < len(words) and words[end] == 0:
                end += 1
            parts.append('{PAD:' + str(end - index) + '}')
            index = end
            continue
        if word in CODE_TO_TOKEN:
            parts.append('{' + CODE_TO_TOKEN[word] + '}')
        elif word in CODE_TO_CHAR:
            parts.append(CODE_TO_CHAR[word])
        elif 0xE38 <= word <= 0xFEF:
            parts.append('{FACE:' + f'{word:04X}' + '}')
        elif 0xE25 <= word <= 0xE2F:
            parts.append('{WAIT:' + str(0xE2F - word) + '}')
        else:
            parts.append('{RAW:' + f'{word:04X}' + '}')
        index += 1
    return ''.join(parts)


def expand_source(source: str, capacities: dict[str, int] | None = None) -> str:
    def replace(match):
        literals = match.group(1)
        if literals is None:
            return match.group()
        text = ''.join(ast.literal_eval(value) for value in re.findall(LITERAL, literals))
        words = encode(text)
        member = re.search(r'\.([A-Za-z_]\w*)\s*=\s*$', source[:match.start()])
        if capacities is not None and member and member.group(1) in capacities:
            capacity = capacities[member.group(1)]
            if len(words) > capacity:
                raise ValueError(f'{member.group(1)} needs {len(words)} glyph words; capacity is {capacity}')
        return '{' + ', '.join(f'0x{word:04X}' for word in words) + '}'
    return SOURCE_TOKEN.sub(replace, source)


def bank_capacities(header: Path) -> dict[str, int]:
    return {name: int(size) for name, size in re.findall(r'\bu16\s+(\w+)\[(\d+)\]', header.read_text())}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('source', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    header = args.source.with_suffix('.h')
    result = expand_source(args.source.read_text(), bank_capacities(header) if header.exists() else None)
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(result, encoding='utf-8', newline='\n')
    else:
        print(result, end='')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
