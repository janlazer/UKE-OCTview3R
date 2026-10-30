[CmdletBinding()]
param(
    [string]$MSBuild = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe',
    [Alias('VolumePath')][string]$TiffPath = ''
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$testOutput = Join-Path $projectRoot 'tmp\gpu-volume-regression'
if ($TiffPath) { $TiffPath = (Resolve-Path -LiteralPath $TiffPath).Path }
New-Item -ItemType Directory -Path $testOutput -Force | Out-Null
& $MSBuild (Join-Path $projectRoot 'OCTview3R.sln') /t:Build /p:Configuration=Release /p:Platform=x64 /m:2 /v:minimal /nologo `
    "/p:ForceImportAfterCppTargets=$PSScriptRoot\gpu-volume-regression.targets" `
    "/p:OutDir=$testOutput\bin\" "/p:IntDir=$testOutput\obj\" /p:TargetName=GpuVolumeRegression
if ($LASTEXITCODE -ne 0) { throw 'GPU regression-test build failed.' }
$names = @('PATH', 'QT_PLUGIN_PATH', 'QT_QPA_PLATFORM')
$saved = @{}
foreach ($name in $names) { $saved[$name] = [Environment]::GetEnvironmentVariable($name, 'Process') }
try {
    $runtime = if (Test-Path "$env:VTKBIN\Release") { "$env:VTKBIN\Release" } else { $env:VTKBIN }
    $env:PATH = "$runtime;$env:QTDIR\bin;$env:PATH"
    $env:QT_PLUGIN_PATH = "$env:QTDIR\plugins"
    $env:QT_QPA_PLATFORM = 'windows'
    $arguments = @{}
    if ($TiffPath) { $arguments.ArgumentList = '"' + $TiffPath + '"' }
    $process = Start-Process -FilePath "$testOutput\bin\GpuVolumeRegression.exe" @arguments `
        -WorkingDirectory $projectRoot -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput "$testOutput\stdout.log" -RedirectStandardError "$testOutput\stderr.log"
    # Bound the test while allowing large local volumes more than one minute.
    $deadline = [DateTime]::UtcNow.AddMinutes(5)
    while (-not $process.WaitForExit(1000)) {
        if ([DateTime]::UtcNow -gt $deadline) {
            Stop-Process -Id $process.Id
            throw 'GPU regression test timed out.'
        }
    }
    $process.WaitForExit()
    Get-Content "$testOutput\stdout.log"
    Get-Content "$testOutput\stderr.log"
    if ($process.ExitCode -ne 0) { throw "GPU regression tests failed (exit $($process.ExitCode))." }
} finally {
    foreach ($name in $names) { [Environment]::SetEnvironmentVariable($name, $saved[$name], 'Process') }
}
