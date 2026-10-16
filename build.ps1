param(
    [ValidateSet('setup', 'split', 'verify', 'match', 'test', 'check')]
    [string]$Task = 'check'
)
$ErrorActionPreference = 'Stop'
$projectPath = $PSScriptRoot
Push-Location -LiteralPath $projectPath
try {
    & (Join-Path $projectPath 'docker/cmd.ps1') make $Task
    if ($LASTEXITCODE -ne 0) { throw "Build task '$Task' failed ($LASTEXITCODE)." }
} finally {
    Pop-Location
}
