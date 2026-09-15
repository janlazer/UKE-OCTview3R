[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$PackageDirectory,
    [string]$RegressionExecutable = (Join-Path $PSScriptRoot '..\tmp\threshold-regression\bin\ThresholdRegression.exe')
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repositoryRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$PackageDirectory = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($PackageDirectory)
$payloadDirectory = Join-Path $PackageDirectory 'payload\OCTview3R'
$manifest = Get-Content -LiteralPath (Join-Path $payloadDirectory 'BUILD-MANIFEST.json') -Raw | ConvertFrom-Json
if (-not $manifest.test_package) { throw 'Only a package built with -TestPackage may be installed by this test.' }
if (-not (Test-Path -LiteralPath $RegressionExecutable -PathType Leaf)) { throw 'Run tests/run-threshold-regression.ps1 first.' }
$setup = Join-Path $PackageDirectory "OCTview3R-$($manifest.version)-windows-x64-setup.exe"
$registration = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\OCTview3R.InstallerSmokeTest.x64_is1'
if (Test-Path -LiteralPath $registration) { throw 'An earlier installer smoke test is still registered. Inspect and uninstall it first.' }
$testRoot = Join-Path $repositoryRoot ('tmp\installer-smoke-' + [Guid]::NewGuid().ToString('N'))
$installDirectory = Join-Path $testRoot 'installed'
New-Item -ItemType Directory -Path $testRoot -Force | Out-Null

function Invoke-TestProcess {
    param([string]$Executable, [string[]]$Arguments)
    $process = Start-Process -FilePath $Executable -ArgumentList $Arguments -WindowStyle Hidden -PassThru
    if (-not $process.WaitForExit(60000)) {
        Stop-Process -Id $process.Id
        throw 'Test installer process timed out.'
    }
    if ($process.ExitCode -ne 0) { throw "Test process failed with exit code $($process.ExitCode)." }
}

function Assert-InstalledPayload {
    foreach ($entry in $manifest.files) {
        $path = Join-Path $installDirectory $entry.path
        if (-not (Test-Path -LiteralPath $path -PathType Leaf)) { throw "Missing installed file: $($entry.path)" }
        if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $entry.sha256) {
            throw "Installed file differs: $($entry.path)"
        }
    }
}

foreach ($iteration in 1, 2) {
    Invoke-TestProcess -Executable $setup -Arguments @('/VERYSILENT', '/SUPPRESSMSGBOXES',
        '/NORESTART', '/NOCLOSEAPPLICATIONS', '/NOICONS',
        ('/DIR="' + $installDirectory + '"'), ('/LOG="' + (Join-Path $testRoot "install-$iteration.log") + '"'))
    if (-not (Test-Path -LiteralPath $registration)) { throw 'Per-user uninstall registration missing.' }
    $registeredDirectory = (Get-ItemProperty -LiteralPath $registration).InstallLocation.TrimEnd('\')
    if ($registeredDirectory -ne $installDirectory) { throw 'Unexpected test install location.' }
    Assert-InstalledPayload
    Write-Host "PASS: installation $iteration and $($manifest.files.Count) payload hashes."
    if ($iteration -eq 1) {
        'Synthetic user file: the uninstaller must preserve this.' |
            Set-Content -LiteralPath (Join-Path $installDirectory 'USER_DATA_PROBE.txt') -Encoding ASCII
    }
}

# The harness uses separate INI preferences and the production Qt/VTK pipelines.
# Run it beside the installed DLLs, without any developer dependency paths.
$installedHarness = Join-Path $installDirectory 'ThresholdRegression.exe'
Copy-Item -LiteralPath $RegressionExecutable -Destination $installedHarness
$environmentNames = @('PATH', 'QTDIR', 'QT_PLUGIN_PATH', 'QT_QPA_PLATFORM', 'QT_QPA_PLATFORM_PLUGIN_PATH', 'VTKDIR', 'VTKLIB', 'VTKBIN')
$savedEnvironment = @{}
foreach ($name in $environmentNames) {
    $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process')
    [Environment]::SetEnvironmentVariable($name, $null, 'Process')
}
try {
    $env:PATH = "$env:SystemRoot\System32;$env:SystemRoot"
    $process = Start-Process -FilePath $installedHarness -WorkingDirectory $testRoot -WindowStyle Hidden -PassThru `
        -RedirectStandardOutput (Join-Path $testRoot 'runtime-stdout.log') `
        -RedirectStandardError (Join-Path $testRoot 'runtime-stderr.log')
    if (-not $process.WaitForExit(60000)) {
        Stop-Process -Id $process.Id
        throw 'Installed-runtime regression timed out.'
    }
    $process.WaitForExit()
    $runtimeOutput = Get-Content -LiteralPath (Join-Path $testRoot 'runtime-stdout.log') -Raw
    Write-Host $runtimeOutput
    if ($process.ExitCode -ne 0 -or $runtimeOutput -notmatch 'PASS: 269') {
        throw 'Installed-runtime regression failed; inspect the logs.'
    }
} finally {
    foreach ($name in $environmentNames) {
        [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name], 'Process')
    }
}

# Validate the exact registered test directory before running its own uninstaller.
$registeredDirectory = (Get-ItemProperty -LiteralPath $registration).InstallLocation.TrimEnd('\')
if ($registeredDirectory -ne $installDirectory -or
    -not $installDirectory.StartsWith($repositoryRoot + '\tmp\', [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Unsafe uninstall target.'
}
$uninstall = Join-Path $installDirectory 'unins000.exe'
Invoke-TestProcess -Executable $uninstall -Arguments @('/VERYSILENT', '/SUPPRESSMSGBOXES',
    '/NORESTART', ('/LOG="' + (Join-Path $testRoot 'uninstall.log') + '"'))
if (Test-Path -LiteralPath $registration) { throw 'Test uninstall registration was not removed.' }
if (Test-Path -LiteralPath (Join-Path $installDirectory 'OCTview3R.exe')) { throw 'Application was not uninstalled.' }
if (-not (Test-Path -LiteralPath (Join-Path $installDirectory 'USER_DATA_PROBE.txt'))) {
    throw 'Uninstaller removed an additional user file.'
}
Write-Host 'PASS: uninstall removed the app/registration and preserved additional user files.'
Write-Host "Test logs and preserved probe files: $testRoot"
