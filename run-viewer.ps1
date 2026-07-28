[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

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
$qtBinDirectory = Join-Path $qtRoot "bin"
$qtPluginDirectory = Join-Path $qtRoot "plugins"
$executable = Join-Path $PSScriptRoot "x64\$Configuration\OCTview3R.exe"

if (-not (Test-Path -LiteralPath $vtkRuntimeDirectory -PathType Container)) {
    throw "VTK runtime directory not found: $vtkRuntimeDirectory"
}
if (-not (Test-Path -LiteralPath $qtBinDirectory -PathType Container)) {
    throw "Qt runtime directory not found: $qtBinDirectory"
}
if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
    throw "OCTview3R executable not found: $executable"
}

$runtimePath = @($vtkRuntimeDirectory, $qtBinDirectory, $env:PATH) -join ";"
$env:PATH = $runtimePath
$env:QT_PLUGIN_PATH = $qtPluginDirectory

& $executable
exit $LASTEXITCODE
