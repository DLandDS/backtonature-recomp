# Native title preview (phase 05, incomplete)

`btn-native --title` compiles the title/menu and new-game setup as host C.
It reads selected image, animation, font and sound resources through the existing
native libraries. It never executes MIPS bytes, invokes PS1 BIOS/CRT0, or casts
an overlay address to a host function pointer.

The extension adds logo/copyright/Start entrance motion, fade transitions,
scrolling scenery, randomized sprite actors and the full new-game setup order.
The baseline d9bc562 dialog opening/closing and input-gating update passes all 19
Windows Debug CTest suites (60.02 seconds, CTest exit 0, user-reported).
The build exit code was not reported separately. Seven additional headless
captures passed and were visually inspected: opening, closing, name entry,
name confirmation, calendar, birthday confirmation and completed setup.
Local evidence: `build/native-validation/phase05-dialog-windows-evidence.json`;
source/data audit: `phase05-dialog-audit.json`; captures: `phase05-dialog-*.bmp/png`.
These local artifacts and retail assets are not committed.
Earlier milestones remain recorded in `docs/phase05-progress.md`.
A successful setup exposes a typed handoff;
farm gameplay, attract gameplay and save/load remain unavailable. PS1 timing,
window/dialog appearance, trig quantization and audio parity are separate gates.
The mayor dialog/portrait/reveal update passes the four title suites before a
wait-icon correction; the corrected build passes targeted synthetic/real-data
portrait/text/wait-indicator checks (build exit 0). Details are recorded below.
The subsequent birthday-header/final-review update passes all four title suites
(256.64 seconds, CTest exit 0) and three additional real-data capture checks.
The follow-up original setup question panels pass all four title suites
(60.15 seconds, CTest exit 0). The other 15 suites were not repeated for this change.
Video observations and the source/data mapping are recorded in
[phase05-reference.md](phase05-reference.md).

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
          (34, 1), (35, 0), (36, 16), (37, 0), (80, 16), (81, 0)]
Path('build/title.input').write_bytes(b''.join(struct.pack('<II', *event) for event in events))
```

For the new entrance/fade sequence, use a replay beginning with `(0,0)`, skip
with `(10,64)`, release at `(11,0)`, and open the menu at `(32,64)` followed by
`(33,0)`. The complete generated setup replay is `setup_replay()` in
`tests/check_title_cli.py`; its current final snapshot is tick 1090. Early presses
during opening are deliberately ignored, so earlier setup replay timings no
longer describe the animated dialog path.
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

The dialog controller ports `menu_box_animate` integer geometry: duration
parameter 12 for opening and 6 for closing, plus six width steps. Including
the source's zero-height frame skip, these take 18 and 12 simulation calls.
The initial zero-width frame produces no raster output. Panels animate from
the centre, close before changing their message, and reveal text/choices only
when fully open. Confirm, cancel and editing presses during animation are
discarded. Pad repeat timing continues, and handoff waits for the final close.
The helpers advance on simulation ticks, so pause and presentation FPS do not
advance them independently.

The audited setup prompts have NO_SCROLL (`0x0FF3`). In `RenderDialogTextPage`
this fills textIndex through END; the supported prompts appear together rather
than using a typewriter effect. No generic scrolling/typing/portrait dialog
engine is claimed. The native help/review text follows the same box lifecycle.

Help text still describes the native keyboard controls. The handoff review,
async resource scheduling and exact PS1 raster/timing remain comparison gates.
Reconstructing window
commands from source is not a hardware reference capture. CLI tests independently
decode both keyboard pages, check their glyph pixels/CLUT, frame edges/gradient,
and reject unsupported or unterminated executable captions. Added checks cover
hand/season pixel decoding, both choice rows, confirmation prompts, hidden
keyboard/calendar, malformed message slots and missing cursor frames.
Dialog unit tests cover integer geometry, open/close counts, blocked input,
held-repeat continuity, reset and handoff gating. Replay checks cover partial
opening/closing frames and complete NO_SCROLL text on the first ready frame.

## Mayor conversation (Windows Debug validated)

The follow-up adds the title overlay's five preface messages in order:
member selected by `menuWindowValues[3]`, slots `0x3A2`, `0x397`, `0x3A5`,
`0x3A4`, `0x3A3`. Welcome has two pages; the other slots ask about the player,
birthday, farm and dog before their setup controls open. The two introductory
messages run once on entering New Game; returning to an earlier setup stage
repeats its question. Presses during the box animation are discarded.

Each slot's first word is portrait page 104, followed by container 0D at +4.
The five portrait banks are identical: 3,644 bytes, CLUT `(0,490,256,1)` and
8bpp image `(960,175,40,80)`. The native loader confines writes to these
rectangles and rejects changed pages, banks or text controls. It reads at
most 128 words from slot +6144. No game text or portrait is embedded in code.
The portrait draws at `(26,84,80,80)` using the original page/CLUT; text starts
at `(30,168)` with spacing `(0,2)`. UI range 7 animates the wait indicator at
`(285,217)`. The conversation uses the undimmed title scenery.

The parser supports only the audited face prefix, glyphs/space/newline,
PAGE_MARK/PAGE_END and END. A/B held while printing selects speed 0 (one
stream word per simulation call); a fresh A/B press advances a page or closes
the completed message. The default interval is read from `textSpeedIntervals[1]`
at `0x80049052`, currently three VBlanks. Each nominal 30 Hz call contributes
two VBlanks; control words can advance before the next glyph interval as in
`ParseDialogTextStream`. Alternating elapsed intervals request text SFX 4.
On keyboard, use Z or X to continue, and hold either to speed up printing.
Return remains the name-grid confirmation key. A small desktop-only control
hint appears below the dialog, outside headless framebuffer captures.
The model advances only in simulation ticks and gates all setup input/handoff.

This is a bounded source reconstruction. Async I/O stalls, exact first-glyph
scheduling, hardware raster/audio and NTSC fractional timing need controlled
PS1 comparison. The mayor-message parser rejects arbitrary faces, inserts,
waits, choices and scrolling past four lines. At this milestone the final
review and birthday header were still native adaptations; the following
section records their source-based follow-up. Native keyboard help remains separate.

New unit checks cover malformed streams, reveal speed, held input at page/end
waits, portrait persistence and handoff gating. CLI checks decode portrait/text
pixels independently, compare partial/full pages and all five messages, replay
setup through tick 1090, and reject damaged portrait/text data. Source/data audit
and Python syntax checks passed. The user reported all four title suites passed
(227.58 seconds, CTest exit 0): unit 0.03, CLI 64.91, tools 0.15, real data
162.48 seconds. The build exit code was not reported separately; the other 15
suites were not repeated.

Capture review then found a wrong wait icon: range 1 showed a heart.
`DrawDialogUiLabelFromArg` defaults to range 7 when textWaitFrames is zero.
The range is corrected and the CLI oracle now checks the indicator itself;
the strengthened synthetic fixture gives ranges 1 and 7 different parts.
The new oracle rejects the old real-data capture. The user rebuilt btn-native
successfully (build exit 0), then two focused 180-frame captures passed: synthetic
and real data, exit 0, with independent portrait/text/range-7 pixel checks.
The real-data result was inspected visually and shows the hourglass. CTest was
not repeated after this range-only correction; the four-suite result above
belongs to the build before it. No C code changed after the successful rebuild.

Local evidence: `build/native-validation/phase05-mayor-windows-evidence.json`,
`phase05-mayor-initial-test.json`, `phase05-mayor-wait-audit.json`, and
`phase05-mayor-wait-{synthetic,retail}.bmp/png`. Earlier partial/page/handoff
captures retain the pre-correction image and are recorded as such.

## Birthday header and final review (Windows Debug validated)

The birthday header reads the complete 14-word caption at `0x80053946`:
punctuation, spaces and slash retain their original positions around the season
icon/day. Calendar and header numbers follow `text_format_number`: single-digit
values have a leading space. Values 10 through 30 retain both digits.

The summary follows overlay function `0x800B9124`: window `(72,56,176,90)`,
text `(76,60)` with spacing `(0,1)`, season icon `(172,72)`, and slash/day
`(188,72)`. The 67-word template comes from executable `0x80051AF6`; a bounded
host expansion supports only PLAYER_NAME, FARM_NAME and DOG_NAME. Each name is
at most eight supported glyphs plus END. Unknown inserts, missing END and
output overflow fail explicitly. No retail text is embedded in source.

Final options come from the text-only slot `0x3AA`, selected through
`menuWindowValues[3]`. The CHOICE_2 prefix and initial newline are consumed;
the two original rows draw at `(30,168)` with spacing `(0,2)`, hand at
`(36,174 + choice*14)`. Both rows appear once the box finishes opening, as in
`text_start_choice`. Arrow keys select; Z/A confirms. X/B and Start do not
cancel this menu, matching `text_choice_menu`. Direction has precedence when
pressed together with A; earlier grid/confirmation input filtering is preserved.
The second choice repeats the player-name question, without the Welcome dialog
or a New Game handoff. The first choice waits for the close/fade before handoff.

New unit checks cover supported name expansion, maximum name lengths, malformed
inserts/output bounds, ignored B/Start, direction precedence and both review
outcomes. CLI pixel checks cover the original summary/choices, both cursor rows,
birthday punctuation/spacing, leading-space day numbers and all 30 calendar days.
The synthetic replay additionally verifies Try again returns to the name question
without exposing handoff data. Invalid summary/caption/choice streams are rejected.

Source/data audit, Python syntax, synthetic resource layout and regression-oracle
checks passed. The new summary oracle rejects the previous native review capture.
The user reports all four title suites passing: native-title 0.03 seconds,
native-title-cli 84.27, native-title-tools 0.14, native-title-data 172.18;
total 256.64 seconds, CTest exit 0. The build exit code was not reported
separately; the supplied script only runs CTest after a successful build.
The other 15 suites were not repeated.

Three additional real-data headless captures passed (exit 0): review/default
choice at frame 1044, second choice at 1046, and reopening the name question
at 1080. Independent summary/text/cursor pixel checks passed for both choices;
the retry log confirms player-name reset and question-message activation with
no handoff. All three captures were inspected visually. Local evidence:
`build/native-validation/phase05-review-windows-evidence.json`,
`phase05-review-audit.json`, and `phase05-review-{ok,retry,name-again}.bmp/png`.
Retail assets and captures stay local.

Native keyboard help, the explicit unavailable-gameplay handoff screen and
controlled PS1 timing/audio comparison remain separate; this update does not
claim playable gameplay or full phase acceptance.
