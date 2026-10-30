[CmdletBinding()]
param(
    [string]$MSBuild = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe',
    [string]$Python = 'python'
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$testOutput = Join-Path $projectRoot 'tmp\oct-regression'
& $Python -B (Join-Path $PSScriptRoot 'make-oct-fixtures.py')
if ($LASTEXITCODE -ne 0) { throw 'Synthetic OCT fixture generation failed.' }
& $MSBuild (Join-Path $projectRoot 'OCTview3R.sln') /t:Build /p:Configuration=Release /p:Platform=x64 /m:2 /v:minimal /nologo `
    "/p:ForceImportAfterCppTargets=$PSScriptRoot\oct-import-regression.targets" `
    "/p:OutDir=$testOutput\bin\" "/p:IntDir=$testOutput\obj\" /p:TargetName=OctImportRegression
if ($LASTEXITCODE -ne 0) { throw 'OCT regression-test build failed.' }
$names = @('PATH','QT_PLUGIN_PATH','QT_QPA_PLATFORM')
$saved = @{}
foreach ($name in $names) { $saved[$name] = [Environment]::GetEnvironmentVariable($name,'Process') }
try {
    $runtime = if (Test-Path "$env:VTKBIN\Release") { "$env:VTKBIN\Release" } else { $env:VTKBIN }
    $env:PATH = "$runtime;$env:QTDIR\bin;$env:PATH"
    $env:QT_PLUGIN_PATH = "$env:QTDIR\plugins"; $env:QT_QPA_PLATFORM = 'windows'
    $process = Start-Process -FilePath "$testOutput\bin\OctImportRegression.exe" -WorkingDirectory $projectRoot `
        -WindowStyle Hidden -PassThru -RedirectStandardOutput "$testOutput\stdout.log" -RedirectStandardError "$testOutput\stderr.log"
    if (-not $process.WaitForExit(60000)) { Stop-Process -Id $process.Id; throw 'OCT regression timed out.' }
    $process.WaitForExit()
    Get-Content "$testOutput\stdout.log"; Get-Content "$testOutput\stderr.log"
    if ($process.ExitCode -ne 0) { throw "OCT regression failed (exit $($process.ExitCode))." }
} finally {
    foreach ($name in $names) { [Environment]::SetEnvironmentVariable($name,$saved[$name],'Process') }
}
