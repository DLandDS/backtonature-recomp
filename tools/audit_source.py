#!/usr/bin/env python3
"""Rebuild the phase-00 lexical source inventory (no SDK or game data needed).

This is an inventory, not a C compiler or a matching gate. It records raw C
definitions, prototypes, includes, calls and ABI review sites, before macros or
conditional compilation. Unknown and indirect calls remain explicitly unknown.
Generated inventories stay under ignored build/; project progress is kept in Notes.
"""
import argparse
import csv
import hashlib
import json
import re
import subprocess
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TOKENS = re.compile(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'', re.S)
SIGNATURE = re.compile(r'^[ \t]*[A-Za-z_][\w \t*]*?\b(\w+)\s*\(([^;{}]*?)\)\s*([;{])', re.M)
CALL = re.compile(r'\b([A-Za-z_]\w*)\s*\(')
KEYWORDS = {'if', 'for', 'while', 'switch', 'sizeof', 'return', 'asm', '__asm__', 'defined'}


def mask(text):
    return TOKENS.sub(lambda m: ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


def line_at(text, offset):
    return text.count('\n', 0, offset) + 1


def closing_brace(text, opening):
    depth = 1
    for i in range(opening + 1, len(text)):
        depth += (text[i] == '{') - (text[i] == '}')
        if depth == 0:
            return i
    raise ValueError('Unclosed function body')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, default=ROOT / 'build/audit-phase-00/inventory')
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    paths = sorted([*ROOT.glob('src/**/*.c'), *ROOT.glob('include/**/*.h')])
    aliases = json.loads((ROOT / 'docs/retail-symbols.json').read_text())
    symbols = {name: int(address, 16) for name, address in re.findall(
        r'^(\w+)\s*=\s*(0x[0-9a-fA-F]+);', (ROOT / 'symbols/symbol_addrs.txt').read_text(), re.M)}

    def address(name):
        symbol = aliases.get(name, name)
        if symbol in symbols:
            return symbols[symbol]
        m = re.fullmatch(r'(?:D_|func_)([0-9A-Fa-f]{8})', symbol)
        return int(m[1], 16) if m else None

    def identity(name):
        addr = address(name)
        return aliases.get(name, name), f'0x{addr:08X}' if addr is not None else ''

    rows = defaultdict(list)
    definitions, prototypes, calls, edges = [], [], {}, defaultdict(list)
    for path in paths:
        rel = path.relative_to(ROOT).as_posix()
        raw = path.read_text()
        code = mask(raw)
        rows['files'].append((rel, hashlib.sha256(path.read_bytes()).hexdigest()))
        spans = []
        for m in SIGNATURE.finditer(code):
            if m[0].lstrip().startswith('typedef ') or m[1] in KEYWORDS:
                continue
            name, signature = m[1], ' '.join(raw[m.start():m.end()-1].split())
            line = line_at(code, m.start())
            if m[3] == '{':
                end = closing_brace(code, m.end()-1)
                nested = any(start <= m.start() <= stop for start, stop in spans)
                spans.append((m.start(), end))
                definitions.append((name, rel, line, line_at(code, end), nested, signature))
                for call in CALL.finditer(code, m.end(), end):
                    if call[1] not in KEYWORDS:
                        calls[(rel, line_at(code, call.start()), call[1])] = name
            else:
                prototypes.append((name, rel, line, signature))
        for m in re.finditer(r'^\s*#include\s+"([^\"]+)"', raw, re.M):
            target = next((p for p in (path.parent / m[1], ROOT / 'include' / m[1]) if p.is_file()), None)
            dest = target.relative_to(ROOT).as_posix() if target else 'UNRESOLVED:' + m[1]
            edges[rel].append(dest)
            rows['includes'].append((rel, line_at(raw, m.start()), dest))
        for m in re.finditer(r'INCLUDE_ASM\("([^\"]+)",\s*(\w+)\)', raw):
            # Only invocations in source, never the macro definition.
            if path.suffix == '.c':
                rows['assembly_includes'].append((rel, line_at(raw, m.start()), m[2], m[1],
                    'retained' if (ROOT / m[1] / (m[2]+'.s')).is_file() else 'requires split'))
        for i, (source_line, code_line) in enumerate(zip(raw.splitlines(), code.splitlines()), 1):
            if re.search(r'\b(?:__asm__|asm)\s*(?:volatile\s*)?\(', code_line):
                rows['inline_assembly'].append((rel, i, source_line.strip()))
            if re.match(r'\s*extern\b', code_line) and ';' in code_line and not any(
                    p[1] == rel and p[2] == i for p in prototypes):
                rows['extern_data'].append((rel, i, source_line.strip()))
            categories = []
            if re.search(r'0x(?:80[01][0-9A-Fa-f]{5}|1[Ff]80[0-9A-Fa-f]{4})\b', code_line):
                categories.append('absolute address')
            if re.search(r'\(\s*(?:s32|u32|u_long)\s*\)', code_line):
                categories.append('32-bit cast: review pointer use')
            if re.search(r'^\s*#define\s+\w*View\b', code_line):
                categories.append('typed alias view')
            if re.search(r'\(\s*(?:s16|u16|s32|u32|u8|char)\s*\*\s*\)\s*\(', code_line):
                categories.append('cast plus offset: review serialized layout')
            if categories:
                rows['abi_review'].append((rel, i, '; '.join(categories), source_line.strip()))

    body_names = {d[0] for d in definitions}
    asm_names = {r[2] for r in rows['assembly_includes']}
    sdk_headers = [p for p in paths if p.name.startswith('psyq_')]
    sdk_names = {m[1] for p in sdk_headers for m in re.finditer(
        r'^\s*#define\s+(\w+)\(', p.read_text(), re.M)}
    sdk_names.update(p[0] for p in prototypes if Path(p[1]) in
                     {p.relative_to(ROOT) for p in sdk_headers})
    def classify(name):
        if name in body_names:
            return 'C body'
        if name in asm_names:
            return 'assembly/data include'
        addr = address(name)
        if addr is not None and 0x80024AE4 <= addr < 0x80048B0C:
            return 'PSY-Q linked region (split required)'
        if addr is not None:
            return 'address-bound; no C body'
        return 'declaration/macro/indirect; unresolved'

    for name, path, line, end, nested, signature in definitions:
        rows['functions'].append((name, *identity(name), path, line, end,
            'nested C' if nested else 'C body', signature))
    for name, path, line, signature in prototypes:
        rows['declarations'].append((name, *identity(name), path, line, classify(name), signature))
    for (path, line, name), caller in sorted(calls.items()):
        rows['calls'].append((path, line, caller, name, *identity(name), classify(name)))
        if name in sdk_names or classify(name).startswith('PSY-Q'):
            rows['sdk_usage'].append((path, line, caller, name, *identity(name),
                'macro/API declaration' if name in sdk_names else 'linked address'))
    for name, symbol in sorted(aliases.items()):
        if name not in body_names:
            rows['address_symbols'].append((name, symbol, identity(name)[1], classify(name)))
    for unit in sorted(ROOT.glob('src/*.c')):
        rel = unit.relative_to(ROOT).as_posix()
        seen = set()
        def visit(path):
            if path in seen:
                return
            seen.add(path)
            for child in edges[path]:
                visit(child)
        visit(rel)
        flags = re.search(r'cc1:\s*(.*?)\s*\*/', unit.read_text())[1]
        for dep in sorted(seen):
            rows['units'].append((rel, flags, dep, 'module' if dep.endswith('.c') else 'header',
                sum(1 for d in definitions if d[1] == dep)))
    headers = {
        'files': ['path', 'sha256'], 'includes': ['path', 'line', 'dependency'],
        'assembly_includes': ['path', 'line', 'symbol', 'directory', 'availability'],
        'inline_assembly': ['path', 'line', 'source'],
        'extern_data': ['path', 'line', 'declaration (lexical candidates)'],
        'abi_review': ['path', 'line', 'review reason', 'source'],
        'functions': ['name', 'retail symbol', 'address', 'path', 'line', 'end line', 'kind', 'signature'],
        'declarations': ['name', 'retail symbol', 'address', 'path', 'line', 'classification', 'signature'],
        'calls': ['path', 'line', 'lexical caller', 'callee', 'retail symbol', 'address', 'classification'],
        'sdk_usage': ['path', 'line', 'lexical caller', 'SDK name', 'retail symbol', 'address', 'evidence'],
        'address_symbols': ['name', 'retail symbol', 'address', 'classification'],
        'units': ['unity entry', 'cc1 flags', 'dependency', 'kind', 'lexical definitions'],
    }
    for name, header in headers.items():
        with (args.output / (name + '.tsv')).open('w', newline='') as out:
            writer = csv.writer(out, delimiter='\t', lineterminator='\n')
            writer.writerow(header)
            writer.writerows(rows[name])
    summary = {'commit': subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(),
               'method': 'lexical, before macro expansion; not a compilation/matching result',
               'counts': {name: len(values) for name, values in sorted(rows.items())},
               'top_level_C': sum(not d[4] for d in definitions),
               'nested_C': sum(d[4] for d in definitions)}
    (args.output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
