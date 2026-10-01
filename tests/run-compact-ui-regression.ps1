[CmdletBinding()]
param(
    [string]$MSBuild = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe',
    [ValidateSet('1', '1.5', '2')][string]$Scale = '1'
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$testOutput = Join-Path $projectRoot 'tmp\compact-ui-regression'
New-Item -ItemType Directory -Path $testOutput -Force | Out-Null
& $MSBuild (Join-Path $projectRoot 'OCTview3R.sln') /t:Build /p:Configuration=Release /p:Platform=x64 /m:2 /v:minimal /nologo `
    "/p:ForceImportAfterCppTargets=$PSScriptRoot\compact-ui-regression.targets" `
    "/p:OutDir=$testOutput\bin\" "/p:IntDir=$testOutput\obj\" /p:TargetName=CompactUiRegression
if ($LASTEXITCODE -ne 0) { throw 'Compact UI test build failed.' }
$environmentNames = @('PATH', 'QT_PLUGIN_PATH', 'QT_QPA_PLATFORM', 'QT_SCALE_FACTOR')
$savedEnvironment = @{}
foreach ($name in $environmentNames) { $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process') }
try {
    $env:PATH = "$env:VTKBIN\Release;$env:QTDIR\bin;$env:PATH"
    $env:QT_PLUGIN_PATH = "$env:QTDIR\plugins"
    $env:QT_QPA_PLATFORM = 'windows'
    $env:QT_SCALE_FACTOR = $Scale
    $process = Start-Process -FilePath "$testOutput\bin\CompactUiRegression.exe" -WorkingDirectory $projectRoot -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput "$testOutput\stdout-$Scale.log" -RedirectStandardError "$testOutput\stderr-$Scale.log"
    if (-not $process.WaitForExit(60000)) { Stop-Process -Id $process.Id; throw 'Compact UI tests timed out.' }
    $process.WaitForExit()
    Get-Content -LiteralPath "$testOutput\stdout-$Scale.log"
    Get-Content -LiteralPath "$testOutput\stderr-$Scale.log"
    if ($process.ExitCode -ne 0) { throw "Compact UI tests failed (exit $($process.ExitCode))." }
} finally {
    foreach ($name in $environmentNames) { [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name], 'Process') }
}
