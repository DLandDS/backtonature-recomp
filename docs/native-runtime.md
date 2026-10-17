# Native SDL3 runtime

This target is a synthetic runtime harness, not a playable BTN port. It links no
retail source, PSY-Q library, overlay, asset or disc data. The existing Make build
and its matching C/headers are unchanged.

## Build on Linux

Requirements: CMake 3.20+, a C11 compiler, Make or Ninja, and Python 3 for tests.
Install your distribution's SDL build prerequisites for desktop video. For X11
on Ubuntu/Debian, the minimal software-renderer setup uses `libx11-dev`,
`libxext-dev`, `libxrandr-dev`, `libxcursor-dev`, `libxi-dev`, `libxfixes-dev`, and
`libxss-dev`. SDL detects optional audio/GPU/Wayland dependencies separately.

```sh
cmake -S . -B build/native -DBTN_BUILD_NATIVE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build/native --parallel
ctest --test-dir build/native --output-on-failure
./build/native/native/btn-native --runtime-test
```

For a machine with no desktop development libraries, configure with
`-DSDL_UNIX_CONSOLE_BUILD=ON` and run with `SDL_VIDEODRIVER=dummy
SDL_RENDER_DRIVER=software`. This verifies headless operation, not a desktop
window, fullscreen behavior or a physical controller.

## Build on Windows

Use Visual Studio 2022 with the Desktop development with C++ workload, CMake,
and Python 3. These commands work in PowerShell:

For a source ZIP, test checklist, and collecting logs, follow
[Windows runtime testing](windows-runtime-test.md). Visual Studio 2022 requires
CMake 3.21 or later.

```powershell
cmake -S . -B build/native -DBTN_BUILD_NATIVE=ON
cmake --build build/native --config Debug --parallel
ctest --test-dir build/native -C Debug --output-on-failure
.\build\native\native\Debug\btn-native.exe --runtime-test
```

SDL 3.2.10 is fetched as a static library from a SHA-256-pinned release archive.
The first configure needs HTTPS access to `codeload.github.com`. Offline builds
can supply a previously verified extraction with
`-DFETCHCONTENT_SOURCE_DIR_SDL3=/path/to/SDL-release-3.2.10`. That override trusts
the supplied source; CMake does not re-check its contents against the archive.

To enable ASan/UBSan for project code on Linux GCC/Clang, add
`-DBTN_SANITIZERS=ON` in a separate build directory. SDL itself is not
instrumented by this option. Unsupported compiler/platform combinations fail
configuration explicitly.

## Controls and configuration

The harness displays a 320x240 CPU framebuffer with nearest-neighbor scaling
and letterboxing. Arrow keys or gamepad D-pad/left stick move the square.
Z/gamepad south is A, X/east is B, and Return is Start. Escape quits; F11 toggles
fullscreen. The on-screen input mask exposes all seven actions.

Configuration uses CLI arguments; preferences are not persisted yet:

```sh
./build/native/native/btn-native --runtime-test --fps 144 --volume 50 \
    --bind up=W --bind down=S --bind left=A --bind right=D \
    --save-dir build/runtime-saves --frames 300
```

`--fps` accepts 1..1000, `--volume` 0..100, and `--frames` 1..1000000. Omitting
`--frames` runs until closed. Bindings use SDL scancode names; Escape/F11 are
reserved. Only one gamepad is active at a time, with automatic replacement on
disconnect. Physical device mappings depend on SDL's gamepad database.

No data directory is needed for synthetic testing. If `--data-dir PATH` is
specified, both `A_FILE.HDT` and `A_FILE.BIN` must be nonempty regular files.
This checks paths only; archive validation belongs to the resource reader.
The separate [archive CLI](native-archive.md) validates HDT/BIN spans and dumps
member ranges without starting the runtime window.
Never overwrite those files. The save directory defaults to SDL's per-user
preference directory. It is created, but no game save is written. Volume is
validated and logged; audio playback is unavailable and no audio device opens.
Failures return nonzero with a diagnostic. Unsupported game execution fails
explicitly instead of returning fake success.

## Timing and portability boundaries

The test simulation uses **synthetic 60 Hz**, independent of presentation FPS
and audio. This is not a claim about BTN's original rate. Focus loss/minimizing
clears input and pauses simulation. Refocusing/restoring resets accumulated
time; held inputs require fresh events. Active stalls process at most 250 ms of
catch-up and log discarded nanoseconds when the application exits.

Input press/release edges survive a tap between ticks and are consumed once per
simulation tick. Keyboard, gamepad buttons, and stick masks remain independent;
disconnecting a controller does not release a key held on the keyboard.

The harness uses C11 fixed-width serialized fields, explicit little-endian reads
and overflow-safe span checks. A PS1 address is a 32-bit value, never a host
pointer. It deliberately does not map PS1 addresses, relocate overlays or resolve
retail function pointers. Those decisions require the source/data audit before
the game source can be linked on a 64-bit host.

## Matching build

The default configuration remains the original matching build, without SDL or
a host C compiler. Use separate build directories when switching modes:

```sh
cmake -S . -B build/cmake
cmake --build build/cmake --target check
```

Native builds default `BTN_BUILD_MATCHING` to OFF. To expose both target sets,
configure with `-DBTN_BUILD_NATIVE=ON -DBTN_BUILD_MATCHING=ON`; the original Make
test target is then named `ps1-test` because CTest owns `test`. Matching still
requires the SDK, retail input and split output documented in the README.
