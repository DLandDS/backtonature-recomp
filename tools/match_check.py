#!/usr/bin/env python3
"""Compile a C translation unit and check each function against the baserom asm.

    python3 tools/match_check.py src/weekday.c

A function "matches" when every instruction word is identical to the one splat
disassembled out of the retail executable. Two operand fields are exempt, and
only two: the %hi and %lo halves of a relocated address, which the linker fills
in and which are therefore zero in a freshly assembled object. Everything else --
register allocation, instruction selection, branch offsets, delay-slot contents
-- has to be exactly right.

Exit status is 0 only if every function in the file matches, so this is usable
as a build gate rather than just a report.
"""
from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))

from tools.asm_order import extract_source_order, place_rodata_after_include, reorder  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent
from tools.source_layout import expand_source

PSYQ_BIN = "/psyq/BIN"
CPP = f"{PSYQ_BIN}/CPPPSX.EXE"
CC1 = f"{PSYQ_BIN}/CC1PSX.EXE"

# Recovered by matching CopyWeekdayAbbreviation, the first matched function.
# Not guesses -- each of these changes
# the output, and these are the values that reproduce retail bytes.
#
# -G is deliberately absent from the defaults. It is a per-translation-unit
# decision: a unit that touches a small global needs a non-zero -G to reach it
# through $gp, and one that does not cannot tell -G0 from -G8. A file states its
# own requirement with a first-line directive:
#
#     /* cc1: -O2 -G8 */
#
# Keeping the flag next to the code that needs it beats a global setting that is
# silently wrong for every unit it does not suit.
CC1_FLAGS = ["-quiet", "-O2"]
ASPSX_VERSION = "2.86"
# -I include is needed as well as cc1's: INCLUDE_ASM expands to a `.include
# "macro.inc"`, so the *assembler* has to find the header too, not just cpp.
AS_FLAGS = ["-EL", "-march=r3000", "-mabi=32", "-no-pad-sections",
            "-I", str(ROOT / "include")]

CC1_DIRECTIVE = re.compile(r"/\*\s*cc1:\s*(.+?)\s*\*/")
INCLUDE_ASM_CALL = re.compile(r"INCLUDE_ASM\s*\(\s*\"[^\"]*\"\s*,\s*(\w+)\s*\)")

# A small extern that retail reaches WITHOUT $gp. cc1 leaves any small symbol
# unsplit (`lw $6,sym`) and records its size in `.extern sym, 4`; GNU as then
# makes it gp-relative. Retail's assembler did not do that for a symbol defined
# in another module: it expanded the macro to `lui $6,%hi(sym)` /
# `lw $6,%lo(sym)($6)`, one register for address and value, adjacent. A
# declaration states this with a trailing marker:
#
#     extern s32 D_8001009C; /* nongp */
#
# and compile_unit drops that symbol's `.extern` size hint before assembly.
# Declaring it as an incomplete array instead is NOT equivalent: cc1 then
# splits the access into HIGH/LO_SUM, which the allocator may give two
# registers, and which the scheduler is free to interleave.
NONGP_DECL = re.compile(
    r"\bextern\b[^;{}]*?\b([A-Za-z_]\w*)\s*(?:\[[^\]]*\]\s*)*;[ \t]*/\*\s*nongp\s*\*/")


def nongp_symbols(source: str) -> set[str]:
    """Symbols whose declarations carry the `/* nongp */` marker."""
    return set(NONGP_DECL.findall(source))


def strip_extern_hints(asm: str, symbols: set[str]) -> str:
    """Remove cc1's `.extern sym, size` lines for the given symbols."""
    if not symbols:
        return asm
    return "".join(
        line for line in asm.splitlines(keepends=True)
        if not ((m := re.match(r"\s*\.extern\s+([A-Za-z_]\w*)\s*,", line))
                and m.group(1) in symbols))

# The small-data base. Derived, not assumed: retail's only gp-relative store so
# far is `sh $v0, 500($gp)` for D_8005E468, giving _gp = 0x8005E468 - 500. Any
# further gp-relative instruction is a check on this value rather than a new
# unknown -- if a second symbol disagrees, this constant is wrong.
GP_BASE = 0x8005E274

GP_REL_SYM = re.compile(r"%gp_rel\(D_([0-9A-Fa-f]{8})\)")
# A data symbol whose name is its address, as a relocation target.
DATA_SYM = re.compile(r"D_([0-9A-Fa-f]{8})")

# j / jal carry a 26-bit absolute target that the linker fills in, so in a fresh
# object it is an offset from the section start instead. The retail operand names
# where it goes, and both local labels and function symbols spell their address
# in the name, so the jump can be checked instead of waived.
JUMP_MNEMONICS = ("j", "jal")

# splat writes the assembled word into the comment: /* fileoff vram WORD */
ASM_LINE = re.compile(
    r"^\s*/\* [0-9A-Fa-f]+ ([0-9A-Fa-f]{8}) ([0-9A-Fa-f]{8}) \*/\s+(\S+)\s*(.*)"
)
OBJDUMP_LINE = re.compile(r"^\s*([0-9a-f]+):\s+([0-9a-f]{8})\s+(\S+)\s*(.*)")
RELOC_LINE = re.compile(r"^\s+[0-9a-f]+:\s+(R_MIPS_\S+)\s+(\S+)")


class MatchError(Exception):
    pass


def _swap32(hexword: str) -> int:
    """splat prints the word little-endian byte-wise; turn it into the value."""
    b = bytes.fromhex(hexword)
    return int.from_bytes(b, "little")


def read_target(asm_path: Path, name: str) -> tuple[list[tuple[int, str, str]], int]:
    """Instructions of one function as (word, mnemonic, operands), plus its vram."""
    out, on, base = [], False, 0
    for line in asm_path.read_text(encoding="utf-8", errors="replace").splitlines():
        if re.match(rf"^glabel {re.escape(name)}\b", line):
            on = True
            continue
        if not on:
            continue
        if line.startswith("endlabel"):
            return out, base
        m = ASM_LINE.match(line)
        if m:
            # group(1) is the vram address, group(2) the assembled word.
            if not out:
                base = int(m.group(1), 16)
            out.append((_swap32(m.group(2)), m.group(3), m.group(4).strip()))
    if on:
        return out, base
    raise MatchError(f"{name} not found in {asm_path}")


def included_asm(c_file: Path) -> set[str]:
    """Functions this unit pulls in as raw assembly rather than compiling."""
    return set(INCLUDE_ASM_CALL.findall(
        expand_source(c_file)))


_REF_INDEX: dict[str, Path] | None = None


def _build_reference_index(asm_dir: Path) -> dict[str, Path]:
    """Map every disassembled function name to the file that defines it.

    Two things move functions around, and both have already broken this once:
    declaring a range as a `c` subsegment moves its functions out of the big
    blob into per-function files under asm/matchings/<unit>/, and splitting a
    subsegment creates further blobs (asm/game_b7e0.s and friends). Indexing
    everything under asm/ once is both faster than re-reading per lookup and
    immune to the next reshuffle.
    """
    index: dict[str, Path] = {}
    pattern = re.compile("^glabel (" + r"\S+" + ")", re.M)
    for path in sorted(asm_dir.glob("*.s")):
        for m in pattern.finditer(path.read_text(encoding="utf-8", errors="replace")):
            index.setdefault(m.group(1), path)
    for path in sorted(asm_dir.glob("matchings/*/*.s")):
        index[path.stem] = path
    return index


def find_reference(name: str, asm_path: Path) -> Path | None:
    """Where the reference disassembly for `name` lives, or None."""
    global _REF_INDEX
    if _REF_INDEX is None:
        _REF_INDEX = _build_reference_index(Path(asm_path).parent
                                            if asm_path.suffix else Path(asm_path))
    return _REF_INDEX.get(name)


def unit_flags(c_file: Path) -> list[str]:
    """Per-unit cc1 flags from a `/* cc1: ... */` directive, else the default."""
    for line in c_file.read_text(encoding="utf-8", errors="replace").splitlines()[:5]:
        m = CC1_DIRECTIVE.search(line)
        if m:
            return ["-quiet", *m.group(1).split()]
    return list(CC1_FLAGS)


# cc1 emits every switch jump table as `.rdata`, `.align 3`, `$Ln:`, `.word`.
# Retail's assembler did not pad those tables to 8 bytes: in the ui unit
# jtbl_80010224 and jtbl_80010378 sit 0x154 bytes apart (4 mod 8), which no
# single 8-aligned section can hold. GNU as honours the 3 and inserts a word
# of padding, shifting the whole linked image, so lower it to the 4-byte
# alignment the tables actually have. Data alignment is left untouched.
JUMP_TABLE_ALIGN = re.compile(r"(\t\.rdata\n\t\.align\t)3(\n\$L\d+:\n\t\.word\t\$L)")


def align_jump_tables(asm: str) -> str:
    """Give cc1 switch tables retail's 4-byte alignment."""
    return JUMP_TABLE_ALIGN.sub(r"\g<1>2\g<2>", asm)


def compile_unit(c_file: Path, build_dir: Path) -> Path:
    """cpp -> cc1 -> maspsx -> as, returning the object path."""
    build_dir.mkdir(parents=True, exist_ok=True)
    stem = c_file.stem
    i_file = build_dir / f"{stem}.i"
    raw_s = build_dir / f"{stem}.raw.s"
    s_file = build_dir / f"{stem}.s"
    o_file = build_dir / f"{stem}.o"

    def run(cmd: list[str], **kw):
        p = subprocess.run(cmd, capture_output=True, text=True, **kw)
        if p.returncode != 0:
            raise MatchError(f"{cmd[0]} failed: {p.stderr.strip() or p.stdout.strip()}")
        return p

    run(["wibo", CPP, "-lang-c", "-I", str(ROOT / "include"),
         str(c_file), str(i_file)])
    flags = unit_flags(c_file)
    run(["wibo", CC1, *flags, str(i_file), "-o", str(raw_s)])
    # cc1 hoists every top-level asm() block ahead of the compiled functions, so
    # a unit mixing C and INCLUDE_ASM comes out in the wrong order. Put it back
    # before maspsx sees it.
    source = expand_source(c_file)
    source_order = extract_source_order(source)
    ordered, undeclared = reorder(raw_s.read_text(encoding="utf-8"), source_order)
    if undeclared:
        raise MatchError(
            f"{c_file}: cc1 emitted {', '.join(undeclared)}, which the source "
            f"order does not mention, so their position in the unit cannot be "
            f"recovered. Every function and INCLUDE_ASM must be visible to "
            f"asm_order.extract_source_order()")
    for predecessor, symbol in re.findall(
            r"/\*\s*rodata-after:\s*(\w+)\s+(\w+)\s*\*/", source):
        try:
            ordered = place_rodata_after_include(ordered, symbol, predecessor)
        except ValueError as exc:
            raise MatchError(f"{c_file}: {exc}") from exc
    ordered = strip_extern_hints(ordered, nongp_symbols(source))
    ordered = align_jump_tables(ordered)
    raw_s.write_text(ordered, encoding="utf-8")
    # maspsx needs the unit's -G as well as cc1: it decides whether a symbol is
    # reached through $gp in one instruction, exposing the load delay, or
    # through an $at expansion that separates the load from its use. Without
    # the limit it treats every symbol as large and drops the nop cc1 asked for.
    gp = [f for f in flags if f.startswith("-G")]
    p = run(["maspsx", "--aspsx-version", ASPSX_VERSION, *gp, str(raw_s)])
    # Make guarded GAS/GTE macros available independently of INCLUDE_ASM
    # placement. This is injected after cc1 and maspsx, so it cannot affect C
    # scheduling or asm-order recovery; repeated includes are guard-protected.
    s_file.write_text('.include "macro.inc"\n' + p.stdout, encoding="utf-8")
    run(["mipsel-linux-gnu-as", *AS_FLAGS, "-o", str(o_file), str(s_file)])
    return o_file


def parse_objdump(output: str) -> tuple[
        dict[str, list[tuple[int, str, str]]],
        dict[str, dict[int, str]],
        dict[str, int]]:
    """Parse `objdump -dr` output without treating local labels as functions."""
    funcs: dict[str, list] = {}
    relocs: dict[str, dict[int, str]] = {}
    starts: dict[str, int] = {}
    cur = curr = None
    for line in output.splitlines():
        m = re.match(r"^([0-9a-f]+) <(\S+)>:", line)
        if m:
            name = m.group(2)
            # GNU objdump prints symbols for jump-table targets as if they were
            # function headers. They are positions inside the current function,
            # so changing owners here truncates that function and invents a
            # fake `.L...` function for the remainder.
            if name.startswith(".L"):
                continue
            # A GNU C nested function is emitted as `name.N`; retail names it
            # without the counter.
            name = re.sub(r"\.\d+$", "", name)
            cur = funcs.setdefault(name, [])
            curr = relocs.setdefault(name, {})
            # Where this function sits inside the object. Jump targets are
            # section-relative, so without this every function after the first
            # in a file appears to jump too far by exactly its own offset.
            starts[name] = int(m.group(1), 16)
            continue
        if cur is None:
            continue
        m = RELOC_LINE.match(line)
        if m and cur:
            curr[len(cur) - 1] = m.group(2)
            continue
        m = OBJDUMP_LINE.match(line)
        if m:
            cur.append((int(m.group(2), 16), m.group(3), m.group(4).strip()))
    return funcs, relocs, starts


def read_object(o_file: Path) -> tuple[
        dict[str, list[tuple[int, str, str]]],
        dict[str, dict[int, str]],
        dict[str, int]]:
    """Disassembly per function, plus relocations and object start offsets.

    `objdump -dr` interleaves relocation lines with the instructions they apply
    to, which is what lets a call to an external symbol be checked by *name*
    rather than by an address the object does not yet know.
    """
    # -z keeps zero words in the disassembly. Without it objdump prints `...`
    # for a run of nops, which silently shortens the parsed function.
    p = subprocess.run(["mipsel-linux-gnu-objdump", "-drz", str(o_file)],
                       capture_output=True, text=True)
    if p.returncode != 0:
        raise MatchError(f"objdump failed: {p.stderr.strip()}")
    return parse_objdump(p.stdout)


def compare(name: str, target: list, actual: list, base: int,
            relocs: dict[int, str] | None = None,
            obj_off: int = 0) -> list[str]:
    """Report every differing instruction. Relocated halves are exempt."""
    problems = []
    if len(target) != len(actual):
        problems.append(
            f"  length: retail has {len(target)} instructions, ours has {len(actual)}")
    for i in range(min(len(target), len(actual))):
        tw, tm, top = target[i]
        aw, am, aop = actual[i]
        if tw == aw:
            continue
        # A relocated lui/addiu carries the address in its low 16 bits, which
        # only the linker can fill in, so the rest of the word is what we check.
        if "%hi" in top or "%lo" in top:
            if (tw >> 16) == (aw >> 16):
                continue
        # A gp-relative displacement is stronger than that: the symbol's own
        # name gives its address, so the value the linker will write is known in
        # advance and can be checked rather than waived.
        m = GP_REL_SYM.search(top)
        if m and (tw >> 16) == (aw >> 16):
            want = (int(m.group(1), 16) - GP_BASE) & 0xFFFF
            if (tw & 0xFFFF) == want:
                continue
            problems.append(
                f"  +0x{i*4:02x}  gp-relative displacement disagrees: retail has "
                f"{tw & 0xFFFF}, D_{m.group(1)} - _gp is {want}")
            continue
        # spimdisasm leaves a %lo as a literal when its %hi was hoisted into a
        # saved register (`addiu $v0, $s4, -0x1790` for %lo(D_8005E870)). Ours
        # relocates against the named symbol, so the linked value is known from
        # the name and is checked, not waived.
        sym = (relocs or {}).get(i)
        m = DATA_SYM.fullmatch(sym or "")
        if m and (tw >> 16) == (aw >> 16):
            addr = int(m.group(1), 16)
            addend = (aw & 0xFFFF) - ((aw & 0x8000) << 1)
            want = ((addr + addend + 0x8000) >> 16 if tm == "lui"
                    else addr + addend) & 0xFFFF
            if (tw & 0xFFFF) == want:
                continue
        # A jump's 26-bit target is section-relative until linked; retail's is
        # absolute. The two agree exactly when the difference between them is
        # this function's own load address, which is knowable, so this is
        # checked rather than waived.
        if tm in JUMP_MNEMONICS and tm == am:
            # A call out of this object resolves by symbol, not by address. The
            # relocation names the callee, so the check is that retail calls the
            # same name -- an address comparison is impossible and unnecessary.
            # A jump to a label inside this same function relocates against the
            # section symbol (".text") with the offset as the addend, which
            # names nothing useful -- fall through to the address check for it.
            sym = (relocs or {}).get(i)
            if sym is not None and not sym.startswith("."):
                if sym in top.split():
                    continue
                problems.append(
                    f"  +0x{i*4:02x}  {tm} calls {sym}, retail calls {top}")
                continue
            retail_to = ((tw & 0x03FFFFFF) << 2) | (base & 0xF0000000)
            ours_off = ((aw & 0x03FFFFFF) << 2) - obj_off
            if retail_to == base + ours_off:
                continue
            problems.append(
                f"  +0x{i*4:02x}  {tm} target disagrees: retail goes to "
                f"0x{retail_to:08X}, ours to base+0x{ours_off:X} "
                f"(= 0x{base + ours_off:08X})")
            continue
        problems.append(
            f"  +0x{i*4:02x}  retail {tw:08x} {tm:<8}{top}\n"
            f"        ours   {aw:08x} {am:<8}{aop}")
    return problems


def main(argv: list[str] | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("c_file")
    ap.add_argument("--asm", default="asm/game.s",
                    help="disassembly holding the reference functions")
    ap.add_argument("--build", default="build/src")
    args = ap.parse_args(argv)

    c_file, asm_path = Path(args.c_file), Path(args.asm)
    try:
        o_file = compile_unit(c_file, Path(args.build))
        ours, relocs, starts = read_object(o_file)
    except MatchError as e:
        print(f"build failed: {e}", file=sys.stderr)
        return 2

    if not ours:
        print(f"{c_file}: no functions produced", file=sys.stderr)
        return 2

    from_asm = included_asm(c_file)
    failed = matched = matched_ins = 0
    for name, actual in sorted(ours.items()):
        ref = find_reference(name, asm_path)
        if ref is None:
            # Never silently pass. A function nobody could check is not a
            # function that is known good.
            failed += 1
            print(f"UNKNOWN {name}: no reference disassembly found "
                  f"(looked in {asm_path} and asm/matchings/*/)")
            continue

        # A function pulled in with INCLUDE_ASM *is* the retail assembly, so it
        # would trivially compare equal. Reporting that as a match would inflate
        # the count with work nobody has done. Count its labeled retail span,
        # not the object symbol extent: handwritten fragments can carry an
        # unlabeled word after endlabel before the next segment begins.
        if name in from_asm:
            target, _ = read_target(ref, name)
            print(f"ASM   {name}  ({len(target)} instructions, not yet C)")
            continue

        target, base = read_target(ref, name)
        problems = compare(name, target, actual, base, relocs.get(name),
                           starts.get(name, 0))
        if problems:
            failed += 1
            print(f"DIFF  {name}  ({len(problems)} difference(s))")
            for p in problems:
                print(p)
        else:
            matched += 1
            matched_ins += len(target)
            print(f"MATCH {name}  ({len(target)} instructions)")

    print(f"      {c_file}: {matched} matched ({matched_ins} instructions), "
          f"{len(from_asm)} still assembly")
    if failed:
        print(f"\n{failed} function(s) do not match or could not be checked",
              file=sys.stderr)
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
