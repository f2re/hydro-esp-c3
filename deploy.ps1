param(
    [switch]$Pull,
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$HydroArgs
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Python = if (Get-Command py -ErrorAction SilentlyContinue) { "py" } else { "python" }

if ($Pull) {
    $Changes = & git -C $Root status --porcelain
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    if ($Changes) {
        Write-Error "[deploy] Local changes detected; commit/stash them before -Pull"
        exit 2
    }

    Write-Host "[deploy] Updating current branch (fast-forward only)"
    & git -C $Root pull --ff-only
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

Write-Host "[deploy] Reusing PlatformIO packages from ~/.platformio and persistent build cache"
& $Python "$Root\tools\hydroctl.py" install @HydroArgs
exit $LASTEXITCODE
