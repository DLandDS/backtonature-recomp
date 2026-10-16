# Docker toolchain

The image carries every Linux-side tool the build needs. Your project checkout
is mounted at `/work`, and your own PSY-Q 4.6 SDK at `/psyq` (read-only).

## Build the image

```sh
docker build -t btnx-decomp:latest docker
```

## Run a command

```powershell
$env:PSYQ_DIR = "C:\path\to\Psyq46"
.\docker\cmd.ps1 make check
```

```sh
PSYQ_DIR=/path/to/Psyq46 ./docker/cmd.sh make check
```

`PSYQ_DIR` is the folder that contains `BIN/CC1PSX.EXE`.

## What is installed

- Base build tools: `git`, `make`, `cmake`, `ninja`, Python 3
- MIPS GNU binutils: `mipsel-linux-gnu-as`, `ld`, `objcopy`, `objdump`, `nm`
- Decomp tools: `splat`, `spimdisasm`, `asm-differ`, `m2c`, `decomp-permuter`
- PS1 helpers: `maspsx` (pinned and patched), `mkpsxiso`, `dumpsxiso`
- `wibo`, which runs the SDK's 32-bit Windows `CC1PSX.EXE` on Linux
- `objdiff-cli` for object diffing

Modern `gcc-mipsel-linux-gnu` is intentionally not installed: C is compiled by
the game's own compiler from your SDK.
