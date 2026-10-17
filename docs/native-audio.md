# Native audio preview (phase 04)

`btn-audio` reads local VAB/SEQ resources or intact XA sectors and produces
stereo PCM. It is a separate preview executable; it does not link retail
overlays or replace the PS1 SDK entry points in a native title scene.
SDL remains pinned to **3.2.10** by the existing CMake configuration.

## Prepare local resources

From the repository root, with Python 3 available:

```powershell
python tools/prepare_native_audio.py --data-dir disc/btn-assets --output-dir build/native-audio
```

The output directory must be new. The exporter validates the pristine US
`SLUS_011.15` hashes before interpreting fixed executable addresses. It reuses
the phase-02 HDT parser and validates every sound span against member 9. The
151-word table at `0x80049370` defines 75 records plus its end sentinel; the
next symbol is `soundDefs` at `0x800495CC`. Each record uses overlapping triples
`[offset[n*2], offset[n*2+1], offset[n*2+2]]`. The middle value is either a VAB
body offset or the sequence marker `1`. It is **not** a monotone offset table.

For the supplied US input, it exports 39 banks and 36 sequences. Each VAB joins
the logical header to the exact sample body; archive sector padding is omitted.
The inventory retains the archive offsets, sample sizes, and padding extents.
The VAB header determines active program count, packed tone blocks, the
256-entry size table, and one-based sample numbers. No signature-only probe is
treated as a complete bank. Exported assets must stay under ignored `build/`.

`audio-inventory.json` also records input SHA-256 hashes, 78 seven-halfword
sound definitions (ending at `voiceStamps`, `0x80049A10`), and the 26 XA track
records between `cdAudioTracks` (`0x80049A80`) and the next symbol
(`0x80049B1C`). The first track field is the retail fade counter, not a sector
length. `cd_audio_fade_tick` decrements it at the game's update rate.

The four executable paths are `\STR\01.XA;1`, `\STR\02.XA;1`,
`\STR\03.XA;1`, and `\STR\04.XA;1`. None is present in the supplied local
asset directory. Presence is checked separately from format/playback validity.
`cd_audio_play_track` selects CD file number **1** and the track's channel;
the track's file field selects which of the four XA paths to read.

Bank/sequence pairing is explicit. Adjacency in the archive does not establish
which bank a scene loads; follow the scene's `sound_queue_seq_pair` calls.

## Build and use

Use the existing native CMake build. Reconfigure after updating the source,
then build `btn-audio` and `btn-audio-test`. Follow the session's Visual Studio
and long-log instructions when running configure/build/tests.

```powershell
.\build\native-vs2026\native\Debug\btn-audio.exe --audio-test --bank build/native-audio/bank-000.vab --sample 1 --seconds 5
.\build\native-vs2026\native\Debug\btn-audio.exe --audio-test --bank build/native-audio/bank-003.vab --sequence build/native-audio/sequence-004.seq --loop --seconds 20
.\build\native-vs2026\native\Debug\btn-audio.exe --audio-test --bank build/native-audio/bank-000.vab --sample 1 --headless --volume 100 --seconds 2 --output build/sample-001.wav
```

The second command is an explicit preview pairing, not proof of a title-scene
association. Output files must be new, including aliases of inputs; exclusive
creation protects existing assets and WAVs. A failed render may leave a partial
new WAV; its nonzero exit status is binding. Headless operation never opens an
audio device or window. Exit codes: 0 success, 1 data/device/I/O failure,
2 invalid arguments. Playback/export is bounded to 1..600 seconds (default 10).

Desktop controls: Space stops/restarts the current selection; left/right select
a sample in sample mode; up/down change volume by 10%; L toggles repeat; Escape
exits. A loop toggle affects sequence loop markers immediately and sample
restart at its next end; toggling off during a hardware-marked sample loop
requires restarting playback. `--fade-ms N` fades to silence over the final N
milliseconds. The fade must fit within the requested duration. A raw sample
preview plays at 44,100 Hz and unit pitch; notes in a sequence use tone tuning.

## XA streaming

Supply a lawful raw extraction with complete sectors and explicitly specify
sector size and file/channel filters:

```powershell
.\build\native-vs2026\native\Debug\btn-audio.exe --audio-test --xa disc/btn-assets/STR/01.XA --sector-size 2352 --file 1 --channel 0 --seconds 10
```

Accepted layouts are 2352-byte raw Mode 2 sectors (including sync/header) and
2336-byte sectors starting at the duplicated XA subheader. The stream preserves
file number, channel, submode, and coding metadata. It skips other file/channel
sectors and non-audio sectors. It decodes 18 sound groups from selected Form 2
audio sectors with persistent, separate left/right predictor histories. Mono
output is duplicated to stereo. Supported coding is 4-bit ADPCM at 37,800 or
18,900 Hz. SDL resamples that PCM for the output device. WAV exports retain
the source sample rate. Loops reset decoder histories at file restart.

2048-byte extraction loses XA audio/subheaders and is rejected. RIFF/CDXA
wrappers, 8-bit coding, emphasis, and format changes within the selected stream
are rejected explicitly. Damaged sync/subheader/header copies and short sectors
are rejected. EDC checking, sector error recovery, and CD hardware zigzag
interpolation are not implemented. Actual XA streaming remains unverified
until the four original media files are available.

## Voice and sequence behavior

The mixer has 24 voices. It chooses a free voice first, otherwise steals the
oldest voice with the lowest eligible tone priority. Higher priorities survive
lower-priority requests. Bank/tone/program volume and pan combine with channel
volume/pan; note-off enters release. Sample repeat markers preserve ADPCM
predictor history across loops. The engine uses an integer sample clock for
SEQ deltas, carries timing fractions across events, supports running status,
program changes, note-on/velocity-zero/note-off, channel volume and pan, and
pitch bend (two-semitone preview range). Sony tempo events have three tempo
bytes without the SMF length byte. Unsupported commands return an error.

The controller audit of the supplied sequences found CC7, CC10, CC99, CC6,
program changes, pitch bend, end markers, and one tempo-change event. Sony
CC99=20 plus CC6 marks the repeat start/count, CC99=30 marks the repeat end;
count 127 means indefinite repeat. These jumps are enabled by `--loop`.
Without it, a sequence traverses the score once. There is a 4096-event budget
per output frame to reject zero-time infinite loops and malformed streams.

Known fidelity limits: nearest-neighbor pitch instead of SPU Gaussian
interpolation, an approximate ADSR rate/counter model, combined linear panning
instead of the SDK's exact volume law, fixed pitch-bend range, and **dry output**.
Banks requesting reverb emit a diagnostic. Vibrato, portamento, exact SDK tone
selection/priority rules, explicit reserved-voice APIs, mono routing, and SDK
scene adapters remain outstanding. This is useful playback, not a claim of
complete SPU/sequencer replacement or title acceptance.

## Threading and buffering

File reads, SEQ parsing, and mixing happen on the producer/main thread. The SDL
device stream has no application callback; the device thread consumes queued
PCM and never accesses a file, bank, or sequence. Voices allocate no memory
during rendering. The producer prebuffers at least 2048 frames before resuming
the device and limits its queue to approximately 3072 frames. Empty queues
after playback starts increment the reported underrun count. Cleanup destroys
the SDL stream before releasing the borrowed resource buffers. This is a
bounded preview queue, not proof of glitch-free behavior under desktop load.

## Tests and acceptance

CTest adds `native-audio`, `native-audio-cli`, and `native-audio-tools`; with
`BTN_NATIVE_DATA_DIR` it also adds `native-audio-data`. The core tests cover
ADPCM signed decoding/predictor/saturation, invalid bank bounds, loop/stop,
volume/fade, clipping/voice stealing, pan/release, sequence chunk invariance and
timing, malformed events, and XA headers/channel layout/coding limits.

The CLI suite checks exclusive WAV output, independent SPU PCM, loop/fade,
bad arguments, synthetic XA file/channel selection, truncated sectors, and
SDL dummy-device queue/drain. The optional local suite parses all 39 banks,
compares each first sample's decoded prefix with an independent Python
recurrence, and renders an explicitly selected bank/sequence pair. This proves
neither all sample playback paths nor PS1 audio equivalence.

Before accepting the whole phase, record a physical/emulated PS1 reference
with the same sample/track, program/tone/note/fine, volume/pan, loop count,
update rate and fade trigger. Compare pitch, duration, loop transition and fade;
retain input hashes, PCM/capture hashes, platform/compiler/SDL version and
known differences. No matching reference capture is currently available.

Format references: [SPU samples/envelopes](https://psx-spx.consoledev.net/soundprocessingunitspu/),
[VAB and SEQ layouts](https://psx-spx.consoledev.net/cdromfileformats/),
[XA sectors](https://psx-spx.consoledev.net/cdromformat/),
[Sony Sound Artist Tool repeat controls](https://psx.arthus.net/sdk/Psy-Q/DOCS/Devrefs/Sound20.pdf),
and [SDL device streams](https://wiki.libsdl.org/SDL3/SDL_OpenAudioDeviceStream).
