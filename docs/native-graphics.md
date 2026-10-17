# Native graphics and resource viewer (phase 03)

`btn-graphics` extends the phase-02 archive reader with a software renderer,
container 0D decoding, animation 0E tables, English glyphs, and a C GTE subset.
It builds with the existing `BTN_BUILD_NATIVE=ON` configuration and pinned
SDL 3.2.10. Matching PS1 sources and headers remain unchanged.

## Run on Windows

From the repository root after building `build/native-vs2026`:

```powershell
# Deterministic synthetic geometry, clipping, blending and depth-order scene.
.\build\native-vs2026\native\Debug\btn-graphics.exe --graphics-test

# Same scene with the English glyph bank from gf.bin.
.\build\native-vs2026\native\Debug\btn-graphics.exe --graphics-test --data-dir disc/btn-assets

# A supplied 8-bit portrait with its 256-color CLUT.
.\build\native-vs2026\native\Debug\btn-graphics.exe --asset-viewer --data-dir disc/btn-assets --member 1 --image 1 --palette 0

# English glyph atlas: the last block of the gf.bin prefix, palette at y=254.
.\build\native-vs2026\native\Debug\btn-graphics.exe --asset-viewer --data-dir disc/btn-assets --member 0 --image 9 --palette 8

# Status animation, with an explicit association to the verified image bank.
.\build\native-vs2026\native\Debug\btn-graphics.exe --asset-viewer --data-dir disc/btn-assets --member 14 --offset 11636 --animation-member 14
```

Linux uses the same arguments with `build/native/native/btn-graphics`.
Left/right selects a block, up/down pans vertically, Space pauses animation,
F11 toggles fullscreen, and Escape exits. Select a palette through `--palette`.
`--depth 4|8|15` overrides the image block's default depth.
`--mode 0|1|2` selects the source decoder's normal/STP/keep-wide-CLUT upload mode.
`--animation-offset`, `--range` and `--frame` choose a particular 0E prefix,
range, and frame. Specifying `--frame` starts paused. Space resumes it.

The diagnostic overlay shows the member/offset, image rectangle/flags/palette,
or animation range/frame. The framebuffer uses nearest-neighbor SDL
presentation. SDL debug text is for diagnostics; the scene's English text
comes from the game pixels, with its own 8x12 glyph layout.

## Verified resource locations in the supplied input

These are **member-relative offsets**, not host addresses or disc sectors.
Input hashes and phase-02 provenance are recorded in
[phase03-progress.md](phase03-progress.md). Other revisions need a fresh audit.

| Member | Resource | 0D offset | First 0D extent / blocks |
|---:|---|---:|---|
| 0 | `objcg\gf.bin` | 0 | 6,904 bytes / 10 blocks |
| 1 | `objcg\face.bin` | 0 | 3,512 bytes / 2 blocks |
| 7 | `data\font\Font_all.tex` | 0 | 62,116 bytes / 3 blocks |
| 14 | `ObjCg\Status.Bin` | 11,636 | after 5 animation prefixes / 183 blocks |
| 18 | `ObjCg\MG1.Bin` | 3,112 | after 5 animation prefixes / 87 blocks |
| 20 | `ObjCg\MG2.Bin` | 4,172 | after 6 animation prefixes / 99 blocks |
| 22 | `ObjCg\MG3.Bin` | 9,020 | after 15 animation prefixes / 156 blocks |
| 24 | `ObjCg\MG4.Bin` | 10,664 | after 4 animation prefixes / 196 blocks |
| 26 | `ObjCg\MG5.Bin` | 7,304 | after 17 animation prefixes / 178 blocks |

All image/CLUT rectangles from the first animation in each of these six
members occur in the selected bank. This supports the explicit sample
associations; it is not a complete inventory of every animation/asset.

The English glyph atlas is **member 0, block 9**, with 16 columns of 8x12
glyphs, at VRAM (896,0), 32 words by 120 rows. The default palette is member 0,
block 8 at (896,254), matching `textClutRows[0]` from the supplied executable.
`Font_all.tex` is a larger Japanese atlas and is not the English text page.
No font pixels or retail text are embedded in tracked source.

## Decoder and renderer contracts

`image_bank.h` parses 0D block headers with explicit little-endian reads. It
accepts unaligned input and keeps serialized lengths separate from host
pointers. Decoding follows `src/graphics/archive_3.c:DecodeContainer0D`:

- raw rows (`flags & 12 == 4`), byte RLE rows, and byte RLE columns (`flags & 8`);
- signed run controls, runs crossing rows/columns, and low/high byte column order;
- separate image/CLUT offsets, mode-2 wide-CLUT placement, and mode-1 STP behavior;
- retail's skipped first word of each STP row and unchanged column-path STP.

Every input/payload/rectangle is bounded. A failed upload leaves VRAM unchanged.
The reader allocates only the selected 0D or 0E prefix, with an explicit 16 MiB
viewer limit. Unknown formats are rejected rather than treated as decoded data.
These functions do not reinterpret TIM or overlay bytes as image banks.

`graphics.h` owns 1024x512 16-bit VRAM, bounded upload/copy/readback, 4/8/15-bit
texture sampling, CLUTs, 256-pixel UV wrapping, texture windows, raw/modulated
colors, four semi-transparency modes, dithering, and mask writes/checks.
CLUT index zero is not inherently transparent: a fetched color of zero is.
Bit 15 controls textured semi-transparency and mask propagation.

Rectangles/sprites, interpolated lines, and flat/Gouraud textured or untextured
triangles/quads draw into that VRAM, respecting clipping and draw offsets.
Interpolation is affine; there is no perspective correction or Z buffer.
Quads split into two triangles with a shared-edge inclusion rule.
Ordering tables copy host primitives into a bounded arena and link **indices**;
descending buckets and head insertion preserve the source ordering convention.
No host pointer is packed into a 24-bit PS1 tag. Overflow, invalid links,
cycles and unsupported texture depths return visible errors.

`animation.h` validates 0E table extents, image/part/frame references and control
targets. It implements the source duration/loop/jump/hold behavior and draws
4/8-bit multi-part frames with flips and semi-transparency. Bank selection is
explicit. The reference scene uses a separate synthetic indexed cube and a
camera/rotation matrix; retail TMD, skeletal models and title scene dispatch
are not integrated.

`gte.h` implements synchronous RTPS/RTPT with sf=1/lm=0, ordinary MVMVA,
NCLIP, AVSZ3/4, FIFOs, MAC wrap, saturation flags, depth cue output and UNR
division. This is a named subset, not a full COP2 interpreter. Color/lighting
commands, reserved matrix selectors and other unimplemented commands have no
success-returning stubs. The scene uses fixed-point, repeatable rotation steps.

Text supports glyphs 00..FF, space (0FFD), newline (FFFE) and END (FFFF).
Other text controls fail with their word offset. Recursive substitutions,
portraits, choices and waits still require their scene/dialog integration.

## Capture and compare

Headless mode opens no window and defaults to one frame. `--tick N` chooses
the synthetic scene's initial simulation tick; it also advances a selected
animation unless `--frame` is given. `--frames N` runs a fixed number of frames.
The interactive clock pauses on focus loss; headless animation advances one
tick per frame. There is no recorded game input stream or RNG in this scene.

```powershell
.\build\native-vs2026\native\Debug\btn-graphics.exe --graphics-test --headless --tick 8 --output build\scene-tick-8.ppm
python tools/compare_graphics.py --reference build\ps1-tick-8.ppm --actual build\scene-tick-8.ppm --report build\comparison.json --diff build\difference.ppm --scene 'same scene and input identity' --tick 8
```

Outputs must not exist. P6 PPM captures contain the unscaled 320x240 software
framebuffer, without SDL diagnostic overlays. The comparison tool accepts P6
PPM with max value 255 and no comments. It refuses different sizes; it never
resizes, aligns or hides discrepancies. It records file/RGB SHA-256, scene/tick,
changed pixels, maximum and mean channel error, plus an absolute RGB diff.
Exit codes: comparison 0 exact / 1 different / 2 invalid; viewer 0 success /
1 resource or rendering failure / 2 invalid arguments.

For a PS1 comparison, first obtain a capture of the **same** scene, resource,
camera, display crop, tick and input sequence. A synthetic native scene does
not by itself supply a corresponding PS1 capture. Python decoder comparisons
establish local decoding consistency, not hardware parity.

## Validation and known differences

CTest adds `native-graphics`, `native-graphics-cli`, and, with
`BTN_NATIVE_DATA_DIR`, `native-graphics-data`. Together with phase 02, that is
9 suites without retail data or **11 suites with it**. Run configure/build and
CTest from [native-runtime.md](native-runtime.md). Per this repository's
session policy, build commands and potentially long test logs run in the
user's terminal; report status/exit code and relevant errors only.

The C suite covers RLE/raw/column streams, signed controls, truncation,
transactional failure, VRAM bounds/overlap, texture depths and transparency,
blending/masks, shared triangle edges, OT order/cycles/capacity, GTE boundary
arithmetic, animation controls/references and text controls. The CLI suite
checks deterministic captures, a 45-degree model rotation, output preservation,
malformed inputs, independent synthetic RGB, comparisons and SDL dummy video.
The optional data suite compares **8 supplied images, 6 animation frames, and
English glyph pixels** against a separate Python decoder/direct table reader.

Known differences requiring PS1 references remain: triangle/line edge and
interpolation rounding, degenerate primitives, transfer wrap and overlapping
GPU-copy behavior, framebuffer feedback/cache effects, GPU timing and a full
GTE command set. CPU VRAM copies use a snapshot, without GPU transfer mask or
wrap semantics. Dithering is implemented but has no captured PS1 golden yet.
Desktop controls/fullscreen need an interactive check; dummy-video tests only
exercise the presentation API. Linux and ASan/UBSan validation are deferred at
the user's request because the laptop's C drive lacks space for WSL. The build
option already exists; no WSL installation is required for the Windows viewer.

Technical references used for the original C implementation:
[project decoder](../src/graphics/archive_3.c),
[animation source](../src/graphics/anim.c),
[glyph source](../src/text/dialog.c),
[PSX GPU specification](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/),
[PSX GTE specification](https://psx-spx.consoledev.net/geometrytransformationenginegte/),
and [SDL texture updates](https://wiki.libsdl.org/SDL3/SDL_UpdateTexture).
No emulator/SDK implementation or retail pixels were copied into this target.
