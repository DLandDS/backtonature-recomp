#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -eq 0 ]; then
  echo "usage: ./docker/cmd.sh <command> [args...]" >&2
  exit 2
fi

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
work_dir="$(cd "$script_dir/.." && pwd)"

# The PSY-Q 4.6 SDK is not distributed with this project. Point PSYQ_DIR at
# your own installation (the folder that contains BIN/CC1PSX.EXE).
mounts=(-v "$work_dir:/work")
if [ -n "${PSYQ_DIR:-}" ] && [ -f "$PSYQ_DIR/BIN/CC1PSX.EXE" ]; then
  mounts+=(-v "$PSYQ_DIR:/psyq:ro")
else
  echo "warning: PSYQ_DIR is not set to a PSY-Q 4.6 SDK -- C compilation and 'make match' will not work." >&2
fi

docker run --rm "${mounts[@]}" -w /work btnx-decomp:latest "$@"
