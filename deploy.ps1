$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Python = if (Get-Command py -ErrorAction SilentlyContinue) { "py" } else { "python" }

$Pull = $false
$DeployArgs = @()
foreach ($Arg in $args) {
    if ($Arg -eq "-Pull" -or $Arg -eq "--pull") {
        $Pull = $true
    } else {
        $DeployArgs += $Arg
    }
}

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

Write-Host "[deploy] Repeat deployment: cached toolchain, stored Wi-Fi/NVS preserved"
& $Python "$Root\tools\usb_deploy.py" @DeployArgs
exit $LASTEXITCODE
