[CmdletBinding()]
param(
    [string]$MSBuild = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
)
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$testOutput = Join-Path $projectRoot "tmp\threshold-regression"
New-Item -ItemType Directory -Path $testOutput -Force | Out-Null
& $MSBuild (Join-Path $projectRoot "OCTview3R.sln") /t:Build /p:Configuration=Release /p:Platform=x64 /m:2 /v:minimal /nologo `
    "/p:ForceImportAfterCppTargets=$PSScriptRoot\threshold-regression.targets" `
    "/p:OutDir=$testOutput\bin\" "/p:IntDir=$testOutput\obj\" /p:TargetName=ThresholdRegression
if ($LASTEXITCODE -ne 0) { throw "Regression-test build failed." }
$originalPath = $env:PATH
$originalPluginPath = $env:QT_PLUGIN_PATH
$originalPlatform = $env:QT_QPA_PLATFORM
$process = $null
try {
    $env:PATH = "$env:VTKBIN\Release;$env:QTDIR\bin;$originalPath"
    $env:QT_PLUGIN_PATH = "$env:QTDIR\plugins"
    $env:QT_QPA_PLATFORM = "windows"
    $process = Start-Process -FilePath "$testOutput\bin\ThresholdRegression.exe" `
        -WorkingDirectory $projectRoot -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput "$testOutput\stdout.log" -RedirectStandardError "$testOutput\stderr.log"
    if (-not $process.WaitForExit(60000)) {
        Stop-Process -Id $process.Id
        throw "Regression test timed out after 60 seconds."
    }
    $process.WaitForExit()
    Get-Content "$testOutput\stdout.log"
    Get-Content "$testOutput\stderr.log"
    if ($process.ExitCode -ne 0) { throw "Regression tests failed (exit $($process.ExitCode))." }
} finally {
    $env:PATH = $originalPath
    $env:QT_PLUGIN_PATH = $originalPluginPath
    $env:QT_QPA_PLATFORM = $originalPlatform
}
