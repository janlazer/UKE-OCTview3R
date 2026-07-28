[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [string]$DestinationRoot = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($DestinationRoot)) {
    $DestinationRoot = Join-Path $PSScriptRoot "dist"
}

function Get-RequiredEnvironmentPath {
    param([string]$Name)

    $value = [Environment]::GetEnvironmentVariable($Name, "Process")
    if ([string]::IsNullOrWhiteSpace($value)) {
        throw "The $Name environment variable is not configured."
    }
    return $value
}

$vtkBinRoot = Get-RequiredEnvironmentPath "VTKBIN"
$qtRoot = Get-RequiredEnvironmentPath "QTDIR"
$vtkRuntimeDirectory = Join-Path $vtkBinRoot $Configuration
$windeployqt = Join-Path $qtRoot "bin\windeployqt.exe"
$sourceExecutable = Join-Path $PSScriptRoot "x64\$Configuration\OCTview3R.exe"
$destinationDirectory = Join-Path $DestinationRoot $Configuration
$destinationExecutable = Join-Path $destinationDirectory "OCTview3R.exe"

if (-not (Test-Path -LiteralPath $vtkRuntimeDirectory -PathType Container)) {
    throw "VTK runtime directory not found: $vtkRuntimeDirectory"
}
if (-not (Test-Path -LiteralPath $windeployqt -PathType Leaf)) {
    throw "windeployqt was not found: $windeployqt"
}
if (-not (Test-Path -LiteralPath $sourceExecutable -PathType Leaf)) {
    throw "Build OCTview3R first. Executable not found: $sourceExecutable"
}

New-Item -ItemType Directory -Path $destinationDirectory -Force | Out-Null
Copy-Item -LiteralPath $sourceExecutable -Destination $destinationExecutable -Force

$vtkDlls = Get-ChildItem -LiteralPath $vtkRuntimeDirectory -Filter "*.dll" -File
if ($vtkDlls.Count -eq 0) {
    throw "No VTK runtime DLLs found in $vtkRuntimeDirectory."
}
$vtkDlls | Copy-Item -Destination $destinationDirectory -Force

$qtBinDirectory = Join-Path $qtRoot "bin"
$env:PATH = @($vtkRuntimeDirectory, $qtBinDirectory, $env:PATH) -join ";"
$env:QT_PLUGIN_PATH = Join-Path $qtRoot "plugins"

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path -LiteralPath $vswhere -PathType Leaf) {
    $visualStudioDirectory = & $vswhere `
        "-latest" `
        "-products" "*" `
        "-requires" "Microsoft.VisualStudio.Component.VC.Tools.x86.x64" `
        "-property" "installationPath"
    if (-not [string]::IsNullOrWhiteSpace($visualStudioDirectory)) {
        $env:VCINSTALLDIR = Join-Path $visualStudioDirectory.Trim() "VC"
    }
}

$qtConfigurationArgument =
    if ($Configuration -eq "Debug") { "--debug" } else { "--release" }
& $windeployqt `
    $qtConfigurationArgument `
    "--compiler-runtime" `
    "--no-translations" `
    "--dir" $destinationDirectory `
    $destinationExecutable
if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed with exit code $LASTEXITCODE."
}

Write-Host "Deployment created: $destinationDirectory"
Write-Host "Copied $($vtkDlls.Count) VTK runtime DLLs."
