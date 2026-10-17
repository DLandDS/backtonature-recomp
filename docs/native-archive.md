# Native archive reader (phase 02)

`btn-archive` is a command-line HDT/BIN reader built with `BTN_BUILD_NATIVE=ON`.
It uses the existing SDL 3.2.10 dependency for
[file I/O](https://wiki.libsdl.org/SDL3/SDL_IOFromFile) without opening a window,
audio device, or controller. The runtime demo and the matching PS1 source are
unchanged. Build instructions are in [native-runtime.md](native-runtime.md).

## List and dump

From the repository root, using an existing Visual Studio Debug build:

```powershell
.\build\native-vs2026\native\Debug\btn-archive.exe --data-dir D:\GameData\BTN --archive-list
.\build\native-vs2026\native\Debug\btn-archive.exe --data-dir D:\GameData\BTN --archive-dump 31 --output build\member-31.bin
.\build\native-vs2026\native\Debug\btn-archive.exe --data-dir D:\GameData\BTN --archive-dump 0 --offset 1 --length 16 --output build\member-0-range.bin
```

Use the actual build directory and your own data location. With a single-config
Linux build the executable is `build/native/native/btn-archive`. Both file names
must be exactly `A_FILE.HDT` and `A_FILE.BIN`. Numbers are unsigned decimal; member
indices are 0..31. Without `--length`, a dump includes the remaining member span.
The output's parent directory must exist, and the output file must be new.
Existing files, including either input and aliases of them, cannot be overwritten.
Output creation uses the C11 exclusive `x` mode, including
[Microsoft's wide-path variant](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-wfopen).
An I/O error may leave an incomplete new dump; its nonzero exit status is binding.

List output is tab-separated: `index`, `offset`, `end`, `span_bytes`. Its first
line reports unclassified HDT extra bytes and BIN prefix/tail lengths. An index
is used as the member identifier until names/flags from a verified executable
are available. No inferred names or payload classifications are emitted.

Exit codes: 0 success, 1 data/I/O failure, 2 invalid CLI arguments. No whole-archive
allocation occurs: only the 132-byte offset table is read at open, and dumps use
a fixed 64 KiB buffer. The library copies exactly the requested bytes into the
caller's buffer, without the retail reader's sector prefix/suffix.

## Format policy and API

The source loader consumes 33 unsigned LE32 offsets, defining 32 allocated spans.
The native reader uses that audited source contract; this is not confirmation of
the original game files. HDT must contain at least 132 bytes. Extra bytes are
accepted and reported, not assumed to be padding. BIN is limited to the audited
32-bit address space (0..4,294,967,295 bytes). Offsets must be nondecreasing and
within BIN. Equal adjacent offsets define an empty member. A nonzero first
offset or a sentinel before EOF is accepted and reported as unclassified bytes.

`archive.h` documents ownership, buffer capacity, zero-length ranges, and failure
behavior. `btn_archive_read` is synchronous and returns success only after the
whole requested range is copied. `btn_archive_read_members` loads consecutive
members directly into host memory, replacing the retail blocking read/copy path
without requiring sector-rounded scratch buffers.

`archive_load.h` supplies the polling adapter for the source loading contracts:

- `btn_archive_read_async`: mode 1 starts/replaces; mode 0 starts if idle or polls
  the existing request. Returns 0 pending, 1 complete, -1 error. A valid restart
  replaces a queued request; invalid requests leave it intact.
- `btn_archive_load_entry_range`: returns the same pending/completion status and
  an explicit result pointer. `copy=1` places the payload at the buffer start;
  `copy=0` places it after the original sector-prefix length and returns that
  pointer. Capacity includes that prefix, which is left untouched.
- `btn_archive_start_range`/`poll_range`/`range_done`: queue consecutive members,
  expose completion once from poll, and keep the completion flag readable.
- `btn_archive_read_idle` and `btn_archive_load_cancel`: inspect pending state
  and cancel before a poll performs any file I/O.

The adapter is single-threaded: its first poll actually reads from the ordinary
file. It does not create a worker, simulate a minimum CD delay, or return success
before the read completes. I/O failures remain visible until an explicit valid
restart instead of reproducing infinite retail retry loops. Requests retain
their archive and destination; polling with a different request is an error.
The archive and buffer must remain alive until completion/cancel. The archive
CLI dump uses this start/poll path for every chunk.

These are host APIs with capacities and host pointers, not binary-compatible PS1
symbols. Their integration into a native game scene belongs to that scene's
port. Rendering/audio CD side effects are not performed by this data reader.
Overlay bytes remain data and are never called as host code.

## Verification

CTest registers `native-archive` (parser/range/buffer/short-read cases) and
`native-archive-cli` (synthetic files, listing, every member's SHA-256 against an
independent direct file read, partial/chunked dumps, invalid inputs, output
preservation). `native-archive-load` covers start/poll/completion, request
replacement/cancel, copy/pointer behavior, consecutive/empty ranges, and sticky
I/O failures. `native-data-tools` runs the six synthetic export/inventory tests.
These extend the existing three runtime tests to seven suites. They need a fresh
configure/build, then:

```powershell
ctest --test-dir build/native-vs2026 -C Debug --output-on-failure
```

With actual game data available, the same independent comparison can be run
locally. It streams one dump at a time and removes it after comparison; allow
enough temporary disk space for the largest allocated member span:

```powershell
python tests/check_archive_cli.py build/native-vs2026/native/Debug/btn-archive.exe D:\GameData\BTN
```

That command validates all 32 span reads against the supplied files. It does not
establish retail identity or validate formats inside members. Record the input
hashes separately using the existing `tools/audit_game_data.py`, with its report
under ignored `build/`. A successful synthetic test is never recorded as a retail
validation result.

For an optional eighth CTest suite, configure with
`-DBTN_NATIVE_DATA_DIR=/absolute/path/to/local/assets`. `native-archive-data` then
repeats the independent 32-member comparison against that directory using the
built CLI and its polling adapter. No retail input is required by default or
copied into CI.

## Executable data and resource inventory

Use the reproducible exporter, which reuses the phase-00 identity checks and
existing text-bank extraction. It requires all three input files and refuses an
existing output directory:

```powershell
python tools/prepare_native_data.py --data-dir disc/btn-assets --output-dir build/native-data-fresh
```

Outputs are local and must remain ignored by Git:

- `member-manifest.json`: 32 verified executable names/raw flags joined with HDT
  spans, input SHA-256 hashes, and preserved raw name bytes. Names are metadata,
  never trusted as extraction paths.
- `text-bank.bin`/`text-bank.json`: the exact 13,406-byte US executable text bank
  and editable 354-entry form. Extraction checks the bank hash and exact encoding
  round-trip before producing either output.
- `executable-data.json`: input identity, load layout, known archive/XA metadata,
  and text-bank layout. PS1 addresses remain serialized addresses, never host
  pointers. Sound-table counts and external XA files are listed as outstanding
  consumer requirements; they are not guessed or represented as available.
- `resource-inventory.json`: bounded probes for all members. Overlay metadata is
  checked before magic bytes so an overlay ID of 0D/0E cannot masquerade as a
  resource. Container 0D headers/block extents, animation 0E table starts, and
  8 KiB message slots are inspected using source-described layouts. Audio/TIM
  signatures are explicitly weaker candidates. Unknown formats stay unknown.

Container probing is separate from the archive reader and does not decode pixels
or play sound. The supplied data has three unclassified member head formats
(event_s, evch, MesEv), plus signature-only audio/TIM probes and unclassified
bytes beyond the first image containers. These are recorded inputs for the
phase-03/04 decoders, not claims of complete payload validation.

The phase-02 acceptance checks cover every checklist item: safe parser/ranges,
file-based loading state, verified metadata/text export, format inventory, CLI,
synthetic failures, and direct-read member hashes. Mark the updated implementation
passed only after rebuilding and exercising the new adapter tests and the
optional supplied-data suite. Full gameplay, native overlay dispatch, graphics
decoding and audio playback remain milestones of later phases.
