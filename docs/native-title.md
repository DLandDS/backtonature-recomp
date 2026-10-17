# Native title preview (phase 05, incomplete)

`btn-native --title` compiles the title/menu and new-game setup as host C.
It reads selected image, animation, font and sound resources through the existing
native libraries. It never executes MIPS bytes, invokes PS1 BIOS/CRT0, or casts
an overlay address to a host function pointer.

The extension adds logo/copyright/Start entrance motion, fade transitions,
scrolling scenery, randomized sprite actors and the full new-game setup order.
The animation/setup and source-based window/text rendering pass all 19
Windows Debug CTest suites (baseline 77ee140: 22.89 seconds).
The shared UI icons/confirmation passes the real-data suite (13.66 seconds)
and synthetic CLI suite (6.01 seconds, exit 0) after a cursor ordering fix and
fixture correction. The other 17 suites passed the initial full run; they were
not repeated after the title drawing/fixture correction.
A successful setup exposes a typed handoff;
farm gameplay, attract gameplay and save/load remain unavailable. PS1 timing,
window/dialog appearance, trig quantization and audio parity are separate gates.

## Run

Build the existing native configuration, then run from the repository root:

```powershell
.\build\native-vs2026\native\Debug\btn-native.exe --title --data-dir disc/btn-assets
```

The directory must contain the audited US `A_FILE.HDT`, `A_FILE.BIN`, and
`SLUS_011.15`. No extracted audio directory or PSY-Q installation is needed.
No game files are included in Git. `--runtime-test` retains its synthetic mode.

Z/Return skips the entrance animation after its initial fade. Return on the
Start prompt opens the menu. Up/down selects New Game (0) or Continue (1); the
initial selection is Continue, matching the overlay reset. Z/Return confirms.
Continue explains that save/load requires phase 07; X/Return dismisses it.
X on the menu returns to Start. This back control is a native preview addition,
not a reconstructed cancel branch from the original two-choice menu.

New Game follows the audited order: player name, birthday, farm name, dog name,
then a final review. Use arrows to move the original 14-column naming cursor,
Z to choose a glyph, Mark/alphabet cell to switch character sets, Space cell for spaces,
X to delete, and Enter/OK cell to request confirmation. Names contain at most
eight glyphs and a bounded END marker. An empty or initially all-space name
cannot be confirmed. Yes accepts; No or X returns to editing. The initial
player-name screen cannot be cancelled with an empty name, matching source.

Birthday selection uses four seasons and the original 30-day calendar wrapping.
X returns from day to season, or season to player naming. Empty-name cancel on
farm/dog naming follows the preceding setup step. Final review Yes produces
`BtnNewGame`: three glyph names, zero-based birthday, four deterministic seed
words and the new-game flag. `btn_title_new_game()` exposes it only once the
transition completes. The screen states that gameplay requires phase 06;
X returns to a reset title and restarts its music. No successful farm scene is
substituted. An idle title's 1800-tick attract request likewise shows the
gameplay boundary instead of pretending to play a demo.

The existing `--bind`, gamepad, focus/minimize, fullscreen, presentation FPS
and volume controls apply. Native actions A/B are Z/X by default. Start alone
opens the prompt; A/Start confirms a menu selection. Directions toggle once
per pressed edge, with precedence over confirm, following `800B86D0`; holding
a direction does not repeat. PS1 Select is not mapped by the current runtime.
The title uses the original logo, text sprites, copyright, cursor, background
and actor assets. Animation cursors advance once per simulated draw call,
including repeated actors that share one source cursor. Presentation renders
cached frames and cannot advance animation or RNG. The host LCG follows
`rand_range`; the initial seed and naming layout tables come from the executable.
Setup text uses the original English glyph atlas. Its native panels, shortened
prompts and transition scheduling are functional adaptations; the retail
dialog/window renderer is not ported in full and visual parity is not claimed.

The simulation uses nominal 30 Hz, inferred from `main_run_frame`'s `VSync(2)`.
It is independent of `--fps`. NTSC fractional timing and PS1 input/capture parity
remain unverified. Asset loading is separate from `BtnTitleState` reset, so
returning to the prompt does not reload an overlay or retain a stale handler.
Audio mixes the mapped bank/sequence and explicit menu SFX tones on the main
thread. SDL's device thread only consumes queued PCM (at most 100 ms ahead).
Focus loss clears input and queued PCM, pauses playback and resets elapsed time.
ADSR, pitch/interpolation, reverb and exact SFX scheduling retain phase 04 limits;
the preview uses dry audio and does not establish PS1 audio parity.

## Provenance and bounded loading

Run the independent audit before using another data revision:

```powershell
python tools/audit_native_title.py --data-dir disc/btn-assets --output build/native-validation/title-audit-new.json
```

The audit gates executable table addresses on the pristine US executable hash,
validates HDT spans and the title dispatch layout, and records all input hashes.
It records structure and addresses, without exporting retail instructions or
pixel/audio bytes into tracked source. Output must not already exist.

| Resource | Location | Evidence |
|---|---|---|
| Title overlay | Member 27, 57,344 bytes; load `0x800B7E20` | Main title scene loads `AFILE_TITLE` into `OVERLAY_LOW` |
| Entry | `0x800B7EEC`, member offset `0xCC` | Existing retail symbol and local overlay dispatch audit |
| Common 0E | Member 0, offset 8,192; 3,356 bytes; 38 ranges | Overlay loader `800BA230` reads this range and binds its animation |
| Images/CLUTs | Member 0, offset 11,548; 139,888 bytes; 73 blocks | Same loader decodes the 0D at read buffer +3,356 |
| Static ranges | 31 logo, 32 copyright, 33 Start, 34 New Game, 35 Continue, 36 cursor | Draw helpers `800B9368` through `800BA118`; independently inspected pixels |
| Title music | Member 9, bank 53 + sequence 54 | `800BA4C4` requests 26; `sound_request.c` indexes executable pair table |
| Menu/setup SFX | Bank 0; sound definitions 0, 1, 2 and 5 | Confirm/cancel/error/move; program/tone/note/fine/volume from executable |
| English font | Member 0, offset 0; glyph page 14, CLUT (896,254) | Existing phase-03 font decoder and `name_entry*.c` |
| Name layout | Executable row/column limits and symbol table | `name_cursor_move`, `name_key_cell_letters`, `name_key_char_symbols` |
| Shared UI 0E/0D | Member 0, animation offset 151,552 (5,084 bytes); image bank offset 156,636 (16,152 bytes, 140 blocks) | All animation image rectangles match the selected bank, with no title/font VRAM overlap |
| Hand cursor / seasons | UI range 11 frame 1 / range 43 frames 0..3 | `ui_icons.c`, `name_marker_draw`, `name_sound_draw`, `name_title_build_display` |
| Name/birthday confirmation | Executable Yes/No at `0x8005175C`; message member from `menuWindowValues[3]`, slots `0x3A6`/`0x3A7`, text at +6,144 | `name_prompt_draw`, `menu_request_open`, `menu_load_result` |

Runtime loaders validate member ranges, entry signatures, animation dimensions,
image streams, executable table spans, VAB extents and sequence structure. They
do not compute full-file cryptographic identities; the independent audit does.
Structural runtime guards are compatibility checks, not proof of byte identity.
All serialized pointers remain LE32 addresses/offsets, never host pointers.

## Repeatable tests and capture

CTest adds `native-title`, `native-title-cli`, `native-title-tools` and, when
`BTN_NATIVE_DATA_DIR` is configured, `native-title-data`. State tests cover
press/hold/precedence, intro/fade, grid bounds/repeat, eight-glyph names,
calendar/cancel/confirmation, handoff, reset, pause and the nominal 30 Hz clock
at 30/60/144 FPS. CLI tests generate synthetic HDT/BIN/EXE,
exercise SDL dummy video/audio, reject malformed replay/input tables and compare
captured animated scenery/title pixels with an independent Python decoder.
They replay the complete setup and verify its final state. Optional real-data
tests repeat the same supported path against local resources. Existing runtime
and audio suites must also run because their shared code is used here.

```powershell
.\build\native-vs2026\native\Debug\btn-native.exe --title --data-dir disc/btn-assets --headless --frames 2 --output build/title-prompt.bmp
```

Headless mode advances one title tick per frame without opening a window or
audio device. It still decodes/mixes audio and fails on an unsupported event.
`--output` writes the last raw framebuffer as BMP; the path must be new. Captures
exclude SDL diagnostic text and unavailable-action panels; use transition logs
to identify state. A screenshot alone never proves a working destination.

`--input-script FILE` is headless-only. Each record is LE32 tick followed by
LE32 held-action mask, beginning at tick 0 with strictly increasing ticks below
`--frames`. The mask persists until the next record; zero releases all keys.
Maximum script size is 1 MiB. Action bits: up=1, down=2, left=4, right=8, A=16,
B=32, Start=64. A held mask is not another press. For example:

```python
import struct
from pathlib import Path
events = [(0, 0), (10, 64), (11, 0), (32, 64), (33, 0),
          (34, 1), (35, 0), (36, 16), (37, 0), (58, 16), (59, 0)]
Path('build/title.input').write_bytes(b''.join(struct.pack('<II', *event) for event in events))
```

For the new entrance/fade sequence, use a replay beginning with `(0,0)`, skip
with `(10,64)`, release at `(11,0)`, and open the menu at `(32,64)` followed by
`(33,0)`. The complete generated setup replay is in `tests/check_title_cli.py`.
Matching PS1 captures and input sequences are still required for phase
acceptance. See [phase05-progress.md](phase05-progress.md).

## Window/text source alignment

The setup uses the bevel and cream gradient from `src/ui/window_frame.c`.
Name and calendar panel coordinates follow `name_entry.c`/`name_entry_2.c`.
Keyboard captions are read as bounded streams from executable addresses
`0x800537B2` and `0x80053888`, with spacing (1,2). This preserves the original
letters/symbols, Mark/alphabet, Space and OK labels rather than rebuilding them.
`runtime/main.c` selects text palette 3; its CLUT row is read from the executable
table at `0x80049044`, matching `text_glyph_sprite`. The existing graphics text
API retains its white default; setup explicitly supplies the dark palette.
The semitransparent black backdrop follows `name_entry_step`/`name_sound_step`.

The setup replaces selection outlines with the original hand sprite and
draws all four season icons, including the selected season in the birthday header.
Name/birthday confirmation reads bounded original prompts and Yes/No text,
places the choice window at (145,76,40,30), and hides the keyboard/calendar
as the source does. UI icons render above text, panels and the dimmed scene.
Only text-only message slots with the expected NO_SCROLL prefix are accepted;
unsupported controls, image-bearing slots and missing END fail visibly.

Help text still describes the native keyboard controls. The handoff review,
dialog opening/typing and exact PS1 raster/timing remain comparison/implementation
gates. Reconstructing window
commands from source is not a hardware reference capture. CLI tests independently
decode both keyboard pages, check their glyph pixels/CLUT, frame edges/gradient,
and reject unsupported or unterminated executable captions. Added checks cover
hand/season pixel decoding, both choice rows, confirmation prompts, hidden
keyboard/calendar, malformed message slots and missing cursor frames.
