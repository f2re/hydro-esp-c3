$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Python = if (Get-Command py -ErrorAction SilentlyContinue) { "py" } else { "python" }
& $Python "$Root\tools\hydroctl.py" update @args
exit $LASTEXITCODE
