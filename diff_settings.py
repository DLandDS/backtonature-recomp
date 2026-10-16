"""Project configuration for asm-differ (`diff.py`), installed in the container.

    .\\docker\\cmd.ps1 bash -lc 'cd /work && make fndiff FUNC=func_80016054'

or directly, for the options this Makefile target does not expose:

    /opt/decomp-python/bin/python -m diff --no-pager func_80016054

Why the linked image and not object files
-----------------------------------------
asm-differ can diff either the linked binaries (`baseimg` vs `myimg`, resolving
function names through `mapfile`) or a pair of object files (`-o`, against an
`expected/` tree). This project uses the binary mode, and does not set
`expected_dir`, on purpose.

`tools/match_check.py` already compares at object level, where relocations are
unresolved. That is the comparison which has three times reported a clean MATCH
for a function that broke the ROM -- by 2, 108 and 209,099 bytes -- because
gp-relative symbol choice, .rodata placement and unit layout are all invisible
until the link. Diffing the linked images means what you read here is the same
thing `make verify` checks.

The cost is that `build/SLUS_011.15` has to exist and be current, so the target
below depends on it. A function still under construction therefore shows up
diffed against a build that includes it; that is the intent.
"""


def apply(config, args):
    config["arch"] = "mipsel"

    # The retail executable and our rebuild of it, plus the map that turns a
    # function name into an address in both.
    config["baseimg"] = "baserom/SLUS_011.15"
    config["myimg"] = "build/SLUS_011.15"
    config["mapfile"] = "build/slus_011.15.map"
    config["map_format"] = "gnu"
    config["build_dir"] = "build/"

    # mipsel-linux-gnu-as assembles this project, so its objdump is the one
    # that agrees with the encodings everywhere else in the toolchain.
    config["objdump_executable"] = "mipsel-linux-gnu-objdump"

    # -m is auto-make in watch mode; `all` is `verify`, which rebuilds the
    # target and checks the SHA-1, so a watch run fails loudly on a regression
    # instead of quietly diffing a stale image.
    config["make_command"] = ["make"]

    config["source_directories"] = ["src", "include", "asm"]
