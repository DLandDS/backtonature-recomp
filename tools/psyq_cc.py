#!/usr/bin/env python3
"""Compile one C translation unit with the game's own toolchain.

    python3 tools/psyq_cc.py src/flags.c --output build/src/flags.o

Thin CLI over the chain in tools/match_check.py, so the build and the match gate
compile through exactly the same path. If they diverged, a function could match
under the gate and still be assembled differently into the executable, which is
the one failure this arrangement must not allow.
"""
from __future__ import annotations

import argparse
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from tools.match_check import MatchError, compile_unit, unit_flags  # noqa: E402


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("c_file")
    ap.add_argument("--output", required=True)
    ap.add_argument("--verbose", action="store_true")
    args = ap.parse_args(argv)

    c_file, out = Path(args.c_file), Path(args.output)
    out.parent.mkdir(parents=True, exist_ok=True)
    try:
        produced = compile_unit(c_file, out.parent)
    except MatchError as e:
        print(f"compile failed: {e}", file=sys.stderr)
        return 1

    if produced.resolve() != out.resolve():
        shutil.copyfile(produced, out)
    if args.verbose:
        print(f"{c_file} -> {out}  [{' '.join(unit_flags(c_file))}]")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
