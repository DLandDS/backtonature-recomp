#!/usr/bin/env python3
"""Restore source order to cc1's output when a unit mixes C and INCLUDE_ASM.

GCC 2.95 does not emit top-level `asm()` blocks where they were written. It
hoists every one of them ahead of the compiled functions, so a unit written as

    func_A   (C)
    func_B   (INCLUDE_ASM)
    func_C   (C)

assembles as B, A, C. For ordinary code that is harmless. For a decompilation it
is fatal: the linker places the unit at a fixed address, so the functions inside
it must appear in the order the retail executable has them, or every address in
the unit after the first is wrong.

The order is recovered from the C source itself, by `extract_source_order()`.
An earlier version of this module instead trusted cc1's own declaration
sequence -- the `.globl` lines and the `#APP` markers -- on the theory that
those stay in source order even when the bodies do not. Live PSY-Q 4.6 output
disproved that. What it actually emits:

  * many INCLUDE_ASM expansions grouped into a single `#APP` region, so the
    markers do not delimit one asm block each;
  * bare blocks with no `#APP` marker at all;
  * `.globl` lines for the C functions grouped together, not interleaved with
    the asm blocks in source order;
  * sometimes no closing `#NO_APP`, after which `.extern`, `.text` and ordinary
    C `.ent`/`.end` bodies simply continue.

Trusting the markers as containers therefore swallowed compiled function bodies
into what looked like one enormous asm block. The markers are now ignored
entirely. The reliable boundary is the INCLUDE_ASM expansion itself, which is
written to be self-contained from `.include "macro.inc"` through the restoring
`.set at`; each such block is one movable body, and each `.ent`/`.end` pair is
another.

`reorder()` takes the source order as its argument. Called without one it falls
back to inferring an order from the `.globl` sequence, which is good enough for
a pure-C unit and for the unit tests, but is not what the build uses.

Limitation: a body whose name never appears in the given order cannot be
placed. It keeps its original relative position and is returned in `undeclared`
so the caller can fail loudly rather than silently mis-place code.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

APP_START = re.compile(r"^\s*#APP\s*$")
APP_END = re.compile(r"^\s*#NO_APP\s*$")
INCLUDED = re.compile(r'\.include\s+"[^"]*?/([A-Za-z_]\w*)\.s"')
ENT = re.compile(r"^\s*\.ent\s+(\S+)")
END = re.compile(r"^\s*\.end\s+(\S+)")
GLOBL = re.compile(r"^\s*\.globl\s+(\S+)")
MACRO_INCLUDE = re.compile(r'^\s*\.include\s+"macro\.inc"\s*$')
SET_AT = re.compile(r"^\s*\.set\s+at\s*$")
# cc1 names a GNU C nested function `name.N`.
NESTED = re.compile(r"^[A-Za-z_]\w*\.\d+$")
SOURCE_INCLUDE = re.compile(
    r'INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*([A-Za-z_]\w*)\s*\)\s*;'
)
SOURCE_FUNCTION = re.compile(
    r"^(?:[A-Za-z_]\w*[ \t*]+)+([A-Za-z_]\w*)[ \t]*"
    r"\([^;{}]*\)[ \t\r\n]*\{",
    re.M,
)


def extract_source_order(source: str) -> list[str]:
    """Read top-level C definitions and INCLUDE_ASM calls in source order."""
    entries = [
        (match.start(), match.group(1))
        for pattern in (SOURCE_INCLUDE, SOURCE_FUNCTION)
        for match in pattern.finditer(source)
    ]
    entries.sort()
    names = [name for _, name in entries]
    if len(names) != len(set(names)):
        duplicates = sorted({name for name in names if names.count(name) > 1})
        raise ValueError(f"duplicate source-order entries: {', '.join(duplicates)}")
    return names


def reorder(text: str, declared_order: list[str] | None = None) -> tuple[str, list[str]]:
    """Return the reordered assembly and the names that had no declaration."""
    lines = text.splitlines()
    preamble: list[str] = []
    bodies: dict[str, list[str]] = {}
    body_seq: list[str] = []      # bodies in the order cc1 emitted them
    infer_order = declared_order is None
    order: list[str] = [] if declared_order is None else list(declared_order)

    def add_asm_body(block: list[str]) -> None:
        m = INCLUDED.search("\n".join(block))
        name = m.group(1) if m else f"__asm_{len(bodies)}"
        bodies[name] = block
        body_seq.append(name)
        if infer_order and name not in order:
            order.append(name)

    i = 0
    while i < len(lines):
        line = lines[i]

        # INCLUDE_ASM is deliberately self-contained from macro.inc through
        # the restoring `.set at`, so that exact block is one movable function
        # body regardless of how cc1 grouped it. This is matched before the
        # markers precisely so the markers never have to be trusted.
        if MACRO_INCLUDE.match(line):
            block = [line]
            i += 1
            while i < len(lines):
                block.append(lines[i])
                if SET_AT.match(lines[i]):
                    i += 1
                    break
                i += 1
            add_asm_body(block)
            continue

        # PSY-Q's markers are not balanced: an assembly-only tail may omit
        # #NO_APP and compiled bodies can follow inside the apparent region.
        # The self-contained INCLUDE_ASM block above is the reliable boundary.
        if APP_START.match(line) or APP_END.match(line):
            i += 1
            continue

        m = ENT.match(line)
        if m:
            name = m.group(1)
            block = [line]
            i += 1
            while i < len(lines):
                block.append(lines[i])
                if END.match(lines[i]):
                    i += 1
                    break
                i += 1
            bodies[name] = block
            body_seq.append(name)
            continue

        m = GLOBL.match(line)
        if infer_order and m and m.group(1) not in order:
            order.append(m.group(1))

        preamble.append(line)
        i += 1

    # A GNU C nested function comes out as `name.N` immediately before the
    # function that contains it, and belongs inside that function's place in
    # the image. Attach each one to the next ordinary body so it moves with it.
    nested: dict[str, list[str]] = {}
    pending: list[str] = []
    for n in body_seq:
        if NESTED.match(n):
            pending.append(n)
        else:
            if pending:
                nested[n] = pending
            pending = []
    body_seq = [n for n in body_seq if not NESTED.match(n)]

    # Anything cc1 never declared (a static function, say) keeps its original
    # position relative to the rest rather than being dropped.
    undeclared = [n for n in body_seq if n not in order]
    for n in undeclared:
        order.insert(body_seq.index(n), n)

    out = list(preamble)
    for name in order:
        block = bodies.get(name)
        if block is None:
            continue  # declared but not defined here: an extern, nothing to place
        for child in nested.get(name, []):
            out.append("\t.text")
            out.extend(bodies[child])
        out.append("\t.text")
        out.extend(block)
    return "\n".join(out) + "\n", undeclared


def place_rodata_after_include(text: str, symbol: str, predecessor: str) -> str:
    """Place one C-emitted rodata definition after an ASM data include.

    cc1 emits file-scope C data before top-level asm, even when the source puts
    it after the asm data. The table and include must stay in one input section
    so the linker can retain their retail order.
    """
    lines = text.splitlines(keepends=True)
    labels = [i for i, line in enumerate(lines) if line.strip() == f"{symbol}:"]
    if len(labels) != 1:
        raise ValueError(f"expected one rodata definition of {symbol}")
    label = labels[0]
    start = label - 1
    while start >= 0 and lines[start].strip() not in (".rdata", ".section .rodata"):
        if lines[start].strip() and not lines[start].lstrip().startswith(".align"):
            raise ValueError(f"cannot isolate rodata definition of {symbol}")
        start -= 1
    if start < 0:
        raise ValueError(f"missing rodata section for {symbol}")
    end = next((i for i in range(label + 1, len(lines))
                if lines[i].strip() == ".text"), None)
    if end is None:
        raise ValueError(f"missing text restore after {symbol}")
    block = lines[start:end + 1]
    del lines[start:end + 1]

    includes = [i for i, line in enumerate(lines)
                if line.strip().endswith(f'/{predecessor}.s"')
                and ".include" in line]
    if len(includes) != 1:
        raise ValueError(f"expected one include of {predecessor}")
    after = next((i for i in range(includes[0] + 1, len(lines))
                  if lines[i].strip() == ".set at"), None)
    if after is None:
        raise ValueError(f"missing ASM restore after {predecessor}")
    lines[after + 1:after + 1] = block
    return "".join(lines)


def main(argv: list[str] | None = None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    if len(argv) != 1:
        print("usage: asm_order.py <file.s>", file=sys.stderr)
        return 2
    p = Path(argv[0])
    text, undeclared = reorder(p.read_text(encoding="utf-8"))
    sys.stdout.write(text)
    for n in undeclared:
        print(f"asm_order: {n} is in no known order; position not recovered",
              file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
