# Native title preview (phase 05, incomplete)

`btn-native --title` compiles a reconstruction of the Start/menu branch as host
C. It reads selected image, animation and sound resources through the existing
native libraries. It never executes MIPS bytes, invokes PS1 BIOS/CRT0, or casts
an overlay address to a host function pointer.

This is a partial title preview. Intro screens, randomized animated scenery,
attract mode, fade transitions and the new-game name-entry flow remain unported.
It does not yet satisfy phase 05 acceptance or lead to playable gameplay.

## Run

Build the existing native configuration, then run from the repository root:

```powershell
.\build\native-vs2026\native\Debug\btn-native.exe --title --data-dir disc/btn-assets
```

The directory must contain the audited US `A_FILE.HDT`, `A_FILE.BIN`, and
`SLUS_011.15`. No extracted audio directory or PSY-Q installation is needed.
No game files are included in Git. `--runtime-test` retains its synthetic mode.

Return opens the menu. Up/down selects New Game (0) or Continue (1); the initial
selection is Continue, matching the overlay reset. Z/Return confirms. An
unsupported destination shows a visible explanation: new-game setup/gameplay
is pending; Continue requires phase 07. X/Return dismisses that explanation.
X on the menu returns to Start. This back control is a native preview addition,
not a reconstructed cancel branch from the original two-choice menu.

The existing `--bind`, gamepad, focus/minimize, fullscreen, presentation FPS
and volume controls apply. Native actions A/B are Z/X by default. Start alone
opens the prompt; A/Start confirms a menu selection. Directions toggle once
per pressed edge, with precedence over confirm, following `800B86D0`; holding
a direction does not repeat. PS1 Select is not mapped by the current runtime.
The Start/menu pixels use the original logo, text sprites, copyright and cursor;
the flat blue background explicitly substitutes for unported scenery.

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
| Menu SFX | Bank 0; sound definitions 0 and 5 | Menu branch requests those IDs; program/tone/note/fine/volume from executable |

Runtime loaders validate member ranges, entry signatures, animation dimensions,
image streams, executable table spans, VAB extents and sequence structure. They
do not compute full-file cryptographic identities; the independent audit does.
Structural runtime guards are compatibility checks, not proof of byte identity.
All serialized pointers remain LE32 addresses/offsets, never host pointers.

## Repeatable tests and capture

CTest adds `native-title`, `native-title-cli`, `native-title-tools` and, when
`BTN_NATIVE_DATA_DIR` is configured, `native-title-data`. State tests cover
press/hold/precedence, unsupported destinations, return/reset, pause and the
nominal 30 Hz clock at 30/60/144 FPS. CLI tests generate synthetic HDT/BIN/EXE,
exercise SDL dummy video/audio, reject malformed replay/input tables and compare
captured sprite pixels with an independent Python decoder. Optional real-data
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
events = [(0, 64), (1, 0), (2, 1), (3, 0), (4, 16), (5, 0), (6, 32)]
Path('build/title.input').write_bytes(b''.join(struct.pack('<II', *event) for event in events))
```

This opens the menu, selects New Game, shows its unavailable state, and returns
to the menu. Matching PS1 captures and input sequences are still required for
phase acceptance. See [phase05-progress.md](phase05-progress.md).
