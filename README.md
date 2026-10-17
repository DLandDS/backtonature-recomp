# backtonature-recomp

A native PC port project for **Harvest Moon: Back to Nature**, based on the
matching decompilation of the original PlayStation game (USA, `SLUS-01115`).

The goal is to run the original game logic on Windows and Linux with minimal
source changes. The existing source folder structure and file locations stay
intact. Replacement PS1 SDK services and host code use C and SDL3.

The matching PS1 build remains a separate reference path. The C in `src/`
rebuilds the main `SLUS_011.15` executable with the original compiler; byte and
per-function checks compare it against the retail executable. The native PC
target does not yet run this source as a complete game.

> **This repository contains no game data.** There is no executable, disc image,
> text, graphics, audio or disassembly of the game here, and no Sony SDK files.
> You supply your own legally obtained game data. The matching PS1 build also
> requires your own PSY-Q SDK; the native tools do not. Everything game-derived
> is generated locally from your copy and is ignored by git.

## Status

### Native PC target

Archive reading, software graphics/GTE, audio previews, and a title/new-game
setup preview are available. These are separate host implementations and tools;
the original game source is not yet linked into a complete native game.

Playable gameplay, game save/load, and full PS1 behavior parity remain
unfinished. Passing a tool or preview test does not establish full game parity.
Detailed progress and the scope of each validation result are maintained in
Notes, linked below.

### Matching PS1 baseline

| | |
|---|---|
| Target | `SLUS_011.15`, sha1 `07fbcffbdd96080d39939e569f84abe76b9d1039` |
| Result | byte-identical rebuild of the main executable |
| C functions | 461 matched by the per-function gate |
| Assembly | `start` and `stup1` (Sony CRT0 bootstrap fragments that GCC 2.95 cannot emit), plus one rodata blob pulled from the split |
| Not covered | the PSY-Q runtime libraries linked into the executable (kept as split assembly from your copy), and the code overlays in `A_FILE.BIN` |

The ten `src/*.c` files are unity-build entry points. Each includes its domain
modules in the original function order, which preserves the translation-unit
boundaries the matching depends on. Compile those ten files, not the modules
on their own.

## Porting approach

- Preserve the original game logic and source layout, changing only what is
  needed for native execution.
- Implement the PS1 SDK services used by the game in a separate compatibility
  layer. SDL3 handles windows, events, controller input, audio output, and
  framebuffer presentation; project code handles PS1-specific behavior.
- Reuse the existing native resource readers, renderer, and audio work while
  connecting original game code to host services.
- Get the original game fully working and validate its behavior before planning
  mod support or adding visual/gameplay enhancements.

## Source layout

| Directory | Contents |
|---|---|
| `src/runtime/` | main loop, initialization, startup |
| `src/cd/` | archive loading, sector reads, unpacking |
| `src/graphics/` | animation, primitives, textures, frame buffers |
| `src/geometry/` | models, camera, collision, vector math |
| `src/input/` | controller input and vibration |
| `src/sound/` | sound requests, loading, sequences, voices |
| `src/text/` | dialog rendering, text layout, text bank layout |
| `src/ui/` | windows, menus, loading screens, UI text |
| `src/game/` | flags, state, tables, gameplay helpers |
| `include/` | shared types; `include/game/` and siblings hold domain types |
| `include/private/` | per-unit declarations, typed views, retail ABI names |
| `include/psyq_*.h` | minimal PSY-Q-compatible interface declarations (see `NOTICE.md`) |
| `asm/nonmatchings/crt0/` | the two hand-kept assembly fragments |
| `config/`, `symbols/` | splat configuration, symbol addresses, linker layout |
| `tools/`, `tests/` | build, matching and text-bank tools, with their tests |
| `docker/` | the toolchain image |
| `native/` | host runtime, resource readers, graphics/audio tools, and title preview |

Readable names map to the original linker symbols through
`include/private/retail_symbols.h` and `docs/retail-symbols.json`.
`docs/identifier-renames.json` records every rename. Unresolved fields keep
their offset names; an unknown meaning is not guessed.

## Documentation

Plans, checklists, phase progress, native build instructions, usage guides, and
validation evidence are maintained in Notes through Dlands Connect:

- [Project plan and checklist](https://notes.dlands.me/doc/rencana-native-port-harvest-moon-back-to-nature-AIFdhztqgl)
- [Combined progress and guides](https://notes.dlands.me/doc/progres-native-port-semua-fase-FMuvm98uBP)

Both documents have clickable contents by phase. Phase documentation is not
duplicated in the repository. Code, tests, and source/symbol metadata remain
here; generated local validation artifacts stay under the ignored `build/`.

For native development, use a C11 compiler, CMake, and Python for the test/tools
workflow. SDL3 is fetched from a pinned release; no PSY-Q installation is
required. The synthetic runtime works without game data. Resource-based
previews require the original files described in Notes.

## Matching PS1 build requirements

1. **The game.** A raw `.bin` (Mode 2/2352) dump of the US disc,
   *Harvest Moon: Back to Nature* (USA), `SLUS-01115`. A good dump of the
   single-track image has sha1 `cb26381fa1336bdd15f07469e59f1143ccf37b03`.
   All the build really needs is the `SLUS_011.15` file on that disc, which
   must hash to the sha1 in the Status table above. Translated or patched
   copies will not match.
2. **The PSY-Q 4.6 SDK**, specifically `BIN/CC1PSX.EXE` and `BIN/CPPPSX.EXE`.
   This is the compiler the game was built with
   (`GNU C 2.95.2 19991024 BUILD 4.0.0030 (PSX)`). It is not distributed here.
3. **Docker**, used to run the Linux toolchain on any host:
   - GNU binutils for `mipsel-linux-gnu` (assembler, linker, objcopy)
   - [splat](https://github.com/ethteck/splat) and spimdisasm, which split
     the retail executable into data and reference assembly
   - [maspsx](https://github.com/mkst/maspsx), pinned and patched
     (`docker/patches/`), which turns cc1 output into GNU as input the way
     the original PSY-Q assembler would
   - [wibo](https://github.com/decompals/wibo), which runs the SDK's 32-bit
     Windows `CC1PSX.EXE` on Linux
   - asm-differ, m2c, decomp-permuter and objdiff for matching work
   - mkpsxiso/dumpsxiso for the optional disc rebuild described below
4. **Python 3.10+** on the host. It is only needed for
   `tools/extract_baserom.py`. Everything else runs in the container.

## Building the matching PS1 executable

### 1. Build the toolchain image

```sh
docker build -t btnx-decomp:latest docker
```

### 2. Provide your baserom

```sh
python3 tools/extract_baserom.py "path/to/Harvest Moon - Back to Nature (USA).bin"
# or, if you already copied the executable off the disc:
python3 tools/extract_baserom.py path/to/SLUS_011.15
```

This writes `baserom/SLUS_011.15`. It refuses to write anything whose hash is
not the US retail executable.

### 3. Tell the scripts where your SDK is

`PSYQ_DIR` is the folder that contains `BIN/CC1PSX.EXE`. It is mounted
read-only into the container at `/psyq`.

```powershell
$env:PSYQ_DIR = "C:\path\to\Psyq46"      # Windows PowerShell
```

```sh
export PSYQ_DIR=/path/to/Psyq46          # Linux / macOS / WSL
```

### 4. Generate the local inputs, then build and check

Use `./docker/cmd.sh` (Linux/macOS/WSL) or `.\docker\cmd.ps1` (Windows):

```sh
./docker/cmd.sh make setup    # split your baserom, extract the text bank
./docker/cmd.sh make check    # build + byte-compare + per-function gate + tests
```

`make setup` runs splat on your baserom. That produces `asm/` (the SDK library
code, data, and reference disassembly that `make match` compares against) and
extracts the game's text into `src/text/text_bank.json`. All of it is
gitignored.

| Target | What it does |
|---|---|
| `make verify` | builds `build/SLUS_011.15` and byte-compares it with the baserom |
| `make match` | compiles every unit and checks each C function against the retail instructions |
| `make test` | runs the Python unit tests |
| `make check` | all of the above, plus forward-declaration and standalone-header checks |
| `make fndiff FUNC=<name>` | per-function assembly diff via asm-differ |
| `make split` | re-run splat after changing `config/` or `symbols/` |
| `make clean` / `make distclean` | remove build output / also remove the split |

On Windows, `.\build.ps1 check` (or `setup`, `verify`, `match`, `test`) wraps
the same commands. CMake offers the same targets
(`cmake -S . -B build/cmake -G Ninja && cmake --build build/cmake --target check`)
but must run inside the container. It delegates to Make. A modern host
compiler is never substituted for GCC 2.95/PSY-Q.

A successful `make verify` ends with:

```
expected: 07fbcffbdd96080d39939e569f84abe76b9d1039  (baserom)
built   : 07fbcffbdd96080d39939e569f84abe76b9d1039  (build)
OK: byte-identical
```

## Editing the text bank

After `make setup`, the game's built-in text bank (menu prompts, ordinals and
similar strings stored inside the executable) is in
**`src/text/text_bank.json`**, decoded from your baserom:

```json
{
  "name": "messages.promptYesNo",
  "offset": 0,
  "capacity": 9,
  "text": " Yes\n No{END}"
}
```

`\n` is a line break and `{END}` is the terminator. Named tokens keep
substitutions such as `{PLAYER_NAME}` and `{FARM_NAME}`, and controls such as
`{PAGE_MARK}` and `{CHOICE_2}`. `{FACE:0E38}` selects a portrait,
`{WAIT:3}` is a timed pause, and `{RAW:1234}` keeps an undecoded glyph
exactly. The codec handles the game's ligatures (`'s`, `'t`, …) and the
apostrophe at `0x008D`.

Each entry has a fixed capacity in glyph words. The build rejects overflow,
missing terminators, overlapping entries and unsupported characters. Only the
text-free layout (`src/text/text_bank.layout.json`, which holds names, offsets
and capacities) is committed. Changing the text deliberately changes the
executable, so `make verify` will then report a mismatch. That is expected.

## Building a playable disc image (BIN/CUE)

The repository has no script for this on purpose: it would have to read and
redistribute files from your disc. The steps are simple to do by hand with
the `mkpsxiso` tools already installed in the image:

1. **Unpack your own disc** into a working folder, outside this repository or
   under a gitignored path such as `build/`:

   ```sh
   ./docker/cmd.sh dumpsxiso -l -x build/disc -s build/disc/layout.xml "disc/Harvest Moon - Back to Nature (USA).bin"
   ```

   `layout.xml` records every file and the volume metadata. `-l` pins each file
   to its original LBA, which the game's sector-based loader relies on.
2. **Replace the executable** with your build:

   ```sh
   ./docker/cmd.sh cp build/SLUS_011.15 build/disc/SLUS_011.15
   ```

   Do not change its size. The layout and the other files (`A_FILE.BIN`,
   `A_FILE.HDT`, XA/STR media and so on) stay exactly as dumped.
3. **Rebuild the image:**

   ```sh
   ./docker/cmd.sh mkpsxiso -y -o build/btn.bin -c build/btn.cue build/disc/layout.xml
   ```

4. Boot `build/btn.cue` in an emulator such as DuckStation.

With an unmodified build, the files inside the new image are identical to the
retail files. The raw image can still differ in mastering details such as EDC/ECC
and padding, which do not affect the game. Do not share the resulting image:
it contains the game.

## Credits

- **[coalslav/hmbtn-decomp](https://github.com/coalslav/hmbtn-decomp)** (CC0
  1.0). Function implementations, type definitions, symbol names and source
  organization came from this project. The full list is in
  [`NOTICE.md`](NOTICE.md).
- The splat, spimdisasm, maspsx, wibo, asm-differ, m2c, decomp-permuter,
  objdiff and mkpsxiso projects, used in the toolchain image.
- The text codec follows the verified font atlas of the BTNx translation
  project.

## Legal

This is an unofficial, non-commercial preservation and research project. It is
not affiliated with or endorsed by Marvelous Inc., Natsume Inc., or Sony
Interactive Entertainment. *Harvest Moon: Back to Nature* is © Marvelous Inc.
(originally Victor Interactive Software); *Harvest Moon* is a trademark of
Natsume Inc. PlayStation and PSY-Q are trademarks of Sony Interactive
Entertainment.

No copyrighted game assets, executables, text, disassembly or SDK files are
included. See [`NOTICE.md`](NOTICE.md) for exactly what the license covers.
