#!/usr/bin/env python3
"""Find intra-unit calls made before the callee is declared.

cc1 falls back to an implicit `int f()` for a callee it has not seen, and that
implicit return keeps $v0 looking live across the call.  When the caller then
stores a literal to more than one place, the allocator puts it in $v1 where
retail used $v0 -- a two-to-four word mismatch with no visible cause in the
source.  func_80023130 and func_800231AC in ui.c were each stuck on exactly
that; one forward declaration matched both.

This lists the call sites where that can still be happening.  It does not say
the callee returns void -- the prototype has to be justified from the callee's
own evidence before it is written down.
"""
import os
import re
import sys
from pathlib import Path
from source_layout import expand_source

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

INCLUDE_ASM_RE = re.compile(r'^\s*INCLUDE_ASM\("[^"]*",\s*([A-Za-z_]\w*)\s*\)')
DEFN_RE = re.compile(r'^[A-Za-z_][\w \t\*]*?([A-Za-z_]\w*)\s*\([^;]*$')
DECL_RE = re.compile(r'^\s*(?:extern\s+)?[A-Za-z_][\w \t\*]*?([A-Za-z_]\w*)\s*\([^)]*\)\s*;')
CALL_RE = re.compile(r'\b([A-Za-z_]\w*)\s*\(')

KEYWORDS = {"if", "while", "for", "switch", "return", "sizeof", "do",
            "INCLUDE_ASM", "__asm__", "defined"}


def main():
    total = 0
    for fn in sorted(os.listdir(os.path.join(ROOT, "src"))):
        if not fn.endswith(".c"):
            continue
        path = os.path.join(ROOT, "src", fn)
        lines = expand_source(Path(path)).splitlines(keepends=True)

        # Deferred functions carry their recorded candidate body in a comment
        # above the stub, calls and all.  Blank comment bodies out rather than
        # dropping the lines, so reported line numbers still point at the file.
        blanked, in_block = [], False
        for line in lines:
            out = line
            if in_block:
                out = "\n" if "*/" not in line else line[line.index("*/") + 2:]
                in_block = "*/" not in line
            elif "/*" in line:
                head = line[:line.index("/*")]
                if "*/" in line[line.index("/*"):]:
                    tail = line[line.index("/*"):]
                    out = head + tail[tail.index("*/") + 2:]
                else:
                    out, in_block = head + "\n", True
            blanked.append(out)
        lines = blanked

        # Where each name in this unit first becomes known to cc1, and where it
        # is defined or stubbed.
        declared = {}
        present = {}
        for i, line in enumerate(lines):
            m = INCLUDE_ASM_RE.match(line)
            if m:
                present.setdefault(m.group(1), i)
                continue
            m = DECL_RE.match(line)
            if m and m.group(1) not in KEYWORDS:
                declared.setdefault(m.group(1), i)
                continue
            m = DEFN_RE.match(line)
            if m and m.group(1) not in KEYWORDS:
                present.setdefault(m.group(1), i)
                declared.setdefault(m.group(1), i)

        hits = []
        for i, line in enumerate(lines):
            # A declaration mentions the name but does not call it, and the
            # stub line is not code either.
            if INCLUDE_ASM_RE.match(line) or DECL_RE.match(line):
                continue
            for name in CALL_RE.findall(line):
                if name in KEYWORDS or name not in present:
                    continue
                if present[name] <= i:
                    continue
                if name in declared and declared[name] < i:
                    continue
                hits.append((i + 1, name))

        if hits:
            seen = {}
            for ln, name in hits:
                seen.setdefault(name, []).append(ln)
            print("%s: %d call site(s), %d callee(s)"
                  % (fn, len(hits), len(seen)))
            for name in sorted(seen, key=lambda n: seen[n][0]):
                where = ", ".join(str(x) for x in seen[name][:6])
                more = "" if len(seen[name]) <= 6 else " ..."
                print("    %-24s called at %s%s  (appears line %d)"
                      % (name, where, more, present[name] + 1))
            total += len(hits)
    print("\n%d forward call site(s) without a visible declaration" % total)
    return 1 if total else 0


if __name__ == "__main__":
    sys.exit(main())
