[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [string]$DestinationRoot = "",

    [switch]$Standalone
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($DestinationRoot)) {
    $DestinationRoot = if ($Standalone) {
        Join-Path $PSScriptRoot "OCTview3R"
    } else {
        Join-Path $PSScriptRoot "dist"
    }
}
if ($Standalone -and $Configuration -ne "Release") {
    throw "Standalone packages must use the Release configuration."
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
$destinationDirectory = Join-Path $DestinationRoot $(if ($Standalone) { "_standalone" } else { $Configuration })
$destinationDirectory = [IO.Path]::GetFullPath($destinationDirectory)
$destinationExecutable = Join-Path $destinationDirectory "OCTview3R.exe"
$zipPath = Join-Path $DestinationRoot "_standalone.zip"

# A fresh standalone directory prevents stale Debug DLLs or older Qt plugins
# from being mixed into a distributable package. Existing packages stay intact.
if ($Standalone) {
    if ((Test-Path -LiteralPath $destinationDirectory) -and
        @(Get-ChildItem -LiteralPath $destinationDirectory -Force).Count -gt 0) {
        throw "Standalone destination is not empty: $destinationDirectory. Move the previous package or choose another -DestinationRoot."
    }
    if (Test-Path -LiteralPath $zipPath) {
        throw "Archive already exists: $zipPath. Move it or choose another -DestinationRoot."
    }
}

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

$releaseDocuments = @(
    @{ Source = (Join-Path $PSScriptRoot "LICENSE"); Destination = "LICENSE.txt" },
    @{ Source = (Join-Path $PSScriptRoot "README.md"); Destination = "README.md" },
    @{ Source = (Join-Path $PSScriptRoot "AUTHORS.md"); Destination = "AUTHORS.md" },
    @{ Source = (Join-Path $PSScriptRoot "CITATION.cff"); Destination = "CITATION.cff" },
    @{ Source = (Join-Path $PSScriptRoot "THIRD_PARTY_NOTICES.md"); Destination = "THIRD_PARTY_NOTICES.md" },
    @{ Source = (Join-Path $PSScriptRoot "OCTview3R\Resources\darkstyle\LICENSE.txt"); Destination = "DARKSTYLE_LICENSE.txt" },
    @{ Source = (Join-Path $PSScriptRoot "packaging\qt.conf"); Destination = "qt.conf" }
)
if ($Standalone) {
    $releaseDocuments += @{ Source = (Join-Path $PSScriptRoot "packaging\START_HERE.txt"); Destination = "START_HERE.txt" }
}
foreach ($document in $releaseDocuments) {
    if (-not (Test-Path -LiteralPath $document.Source -PathType Leaf)) {
        throw "Required release document not found: $($document.Source)"
    }
    Copy-Item -LiteralPath $document.Source `
        -Destination (Join-Path $destinationDirectory $document.Destination) `
        -Force
}

$vtkDlls = @(Get-ChildItem -LiteralPath $vtkRuntimeDirectory -Filter "*.dll" -File |
    Where-Object { $_.Name -ne "QVTKWidgetPlugin.dll" })
if ($vtkDlls.Count -eq 0) {
    throw "No VTK runtime DLLs found in $vtkRuntimeDirectory."
}
$vtkDlls | Copy-Item -Destination $destinationDirectory -Force

$qtBinDirectory = Join-Path $qtRoot "bin"
$env:PATH = @($vtkRuntimeDirectory, $qtBinDirectory, $env:PATH) -join ";"
$env:QT_PLUGIN_PATH = Join-Path $qtRoot "plugins"

$vswhere = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
$visualStudioDirectory = ""
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
# Scan the VTK Qt modules too (for example Qt SQL and OpenGL), not only
# the modules imported directly by the viewer executable.
$qtVtkModules = @($vtkDlls | Where-Object { $_.Name -like "*Qt*" } |
    ForEach-Object { Join-Path $destinationDirectory $_.Name })
& $windeployqt `
    $qtConfigurationArgument `
    "--compiler-runtime" `
    "--no-translations" `
    "--dir" $destinationDirectory `
    $destinationExecutable @qtVtkModules
if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed with exit code $LASTEXITCODE."
}

if ($Standalone) {
    # Use the redistributable runtime files, never DLLs from System32 or
    # development/debug directories. Windows 10/11 supplies the Universal CRT.
    if ([string]::IsNullOrWhiteSpace($visualStudioDirectory)) {
        throw "Visual Studio C++ redistributable files are required for standalone packaging."
    }
    $redistVersionFile = Join-Path $visualStudioDirectory.Trim() "VC\Auxiliary\Build\Microsoft.VCRedistVersion.default.txt"
    $redistVersion = (Get-Content -LiteralPath $redistVersionFile -Raw).Trim()
    $redistRoot = Join-Path $visualStudioDirectory.Trim() "VC\Redist\MSVC\$redistVersion\x64"
    foreach ($component in @("Microsoft.VC143.CRT", "Microsoft.VC143.OpenMP")) {
        $componentDirectory = Join-Path $redistRoot $component
        $runtimeDlls = @(Get-ChildItem -LiteralPath $componentDirectory -Filter "*.dll" -File)
        if ($runtimeDlls.Count -eq 0) { throw "Missing redistributable component: $componentDirectory" }
        $runtimeDlls | Copy-Item -Destination $destinationDirectory -Force
    }

    $qtLicenses = [IO.Path]::GetFullPath((Join-Path $qtRoot "..\..\Licenses"))
    if (Test-Path -LiteralPath $qtLicenses -PathType Container) {
        $licenseDestination = Join-Path $destinationDirectory "licenses\qt-distribution"
        New-Item -ItemType Directory -Path $licenseDestination -Force | Out-Null
        Get-ChildItem -LiteralPath $qtLicenses -File |
            Copy-Item -Destination $licenseDestination -Force
    }

    foreach ($required in @("OCTview3R.exe", "Qt5Core.dll", "Qt5Gui.dll", "Qt5Widgets.dll",
        "Qt5Svg.dll", "platforms\qwindows.dll", "imageformats\qsvg.dll", "iconengines\qsvgicon.dll",
        "vcruntime140.dll", "vcruntime140_1.dll", "msvcp140.dll", "vcomp140.dll")) {
        if (-not (Test-Path -LiteralPath (Join-Path $destinationDirectory $required) -PathType Leaf)) {
            throw "Standalone package is missing $required"
        }
    }

    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [IO.Compression.ZipFile]::CreateFromDirectory(
        $destinationDirectory, [IO.Path]::GetFullPath($zipPath),
        [IO.Compression.CompressionLevel]::Optimal, $true)
    Write-Host "Standalone ZIP created: $zipPath"
}

Write-Host "Deployment created: $destinationDirectory"
Write-Host "Copied $($vtkDlls.Count) VTK runtime DLLs."
Write-Host "Included application and third-party license notices."
