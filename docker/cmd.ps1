$ErrorActionPreference = "Stop"

if ($args.Count -eq 0) {
  throw "usage: .\docker\cmd.ps1 <command> [args...]"
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$workDir = Resolve-Path (Join-Path $scriptDir "..")

# C is compiled with the game's own compiler, CC1PSX.EXE from PSY-Q 4.6, run
# under wibo. The SDK is not distributed with this project; point PSYQ_DIR at
# your own installation (the folder that contains BIN\CC1PSX.EXE). It is
# mounted read-only at /psyq.
$psyq = $env:PSYQ_DIR
$mounts = @("-v", "${workDir}:/work")
if ($psyq -and (Test-Path (Join-Path $psyq "BIN\CC1PSX.EXE"))) {
  $mounts += @("-v", "${psyq}:/psyq:ro")
} else {
  Write-Warning "PSYQ_DIR is not set to a PSY-Q 4.6 SDK -- C compilation and 'make match' will not work."
}

docker run --rm @mounts -w /work btnx-decomp:latest @args
exit $LASTEXITCODE
