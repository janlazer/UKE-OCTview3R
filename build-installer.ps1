[CmdletBinding()]
param(
    [string]$StandaloneDirectory = (Join-Path $PSScriptRoot 'OCTview3R\_standalone'),
    [string]$OutputDirectory = (Join-Path $PSScriptRoot 'dist\installer'),
    [string]$ISCC = '',
    [switch]$AllowDirty,
    [switch]$TestPackage
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repositoryRoot = [IO.Path]::GetFullPath($PSScriptRoot)
$StandaloneDirectory = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($StandaloneDirectory)
$OutputDirectory = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDirectory)

function Invoke-RepositoryGit {
    param([string[]]$Arguments)
    $result = & git -c "safe.directory=$($repositoryRoot.Replace('\', '/'))" -C $repositoryRoot @Arguments
    if ($LASTEXITCODE -ne 0) { throw "Git failed: $($Arguments[0])" }
    return $result
}

$sourceState = @(Invoke-RepositoryGit -Arguments @('status', '--porcelain'))
if ($sourceState.Count -gt 0 -and -not $AllowDirty) {
    throw 'Commit the release sources first, or use -AllowDirty for a local preview only.'
}
$commit = (Invoke-RepositoryGit -Arguments @('rev-parse', 'HEAD')).Trim()
$mainSource = Get-Content -LiteralPath (Join-Path $repositoryRoot 'OCTview3R\main.cpp') -Raw
$versionMatch = [regex]::Match($mainSource, 'setApplicationVersion\(QStringLiteral\("(\d+\.\d+\.\d+)"\)\)')
if (-not $versionMatch.Success) { throw 'Cannot determine application version from main.cpp.' }
$version = $versionMatch.Groups[1].Value
if ((Test-Path -LiteralPath $OutputDirectory) -and
    @(Get-ChildItem -LiteralPath $OutputDirectory -Force).Count -gt 0) {
    throw 'Output directory is not empty. Use a fresh -OutputDirectory; existing releases are never overwritten.'
}
if ([string]::IsNullOrWhiteSpace($ISCC)) {
    $compiler = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($compiler) { $ISCC = $compiler.Source }
}
if (-not $ISCC -or -not (Test-Path -LiteralPath $ISCC -PathType Leaf)) {
    throw 'Provide the Inno Setup 7.1+ compiler using -ISCC <path-to-ISCC.exe>.'
}
$compilerVersion = & $ISCC --version
if ($LASTEXITCODE -ne 0 -or $compilerVersion -notmatch '^7\.\d+\.\d+$' -or [version]$compilerVersion -lt [version]'7.1.0') {
    throw 'This installer requires Inno Setup 7.1 or later in the 7.x series.'
}

$requiredFiles = @('OCTview3R.exe', 'Qt5Core.dll', 'Qt5Gui.dll', 'Qt5Widgets.dll',
    'Qt5OpenGL.dll', 'Qt5Svg.dll', 'platforms\qwindows.dll', 'imageformats\qsvg.dll',
    'iconengines\qsvgicon.dll', 'vtkCommonCore-8.2.dll', 'vtkRenderingOpenGL2-8.2.dll',
    'vtkRenderingVolumeOpenGL2-8.2.dll', 'vcruntime140.dll', 'vcruntime140_1.dll',
    'msvcp140.dll', 'vcomp140.dll', 'qt.conf', 'LICENSE.txt', 'THIRD_PARTY_NOTICES.md')
foreach ($relativePath in $requiredFiles) {
    if (-not (Test-Path -LiteralPath (Join-Path $StandaloneDirectory $relativePath) -PathType Leaf)) {
        throw "Standalone package is missing $relativePath"
    }
}
$inputFiles = @(Get-ChildItem -LiteralPath $StandaloneDirectory -Recurse -Force)
if (@($inputFiles | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }).Count) {
    throw 'Reparse points are not permitted in the payload.'
}
if (@($inputFiles | Where-Object { -not $_.PSIsContainer -and $_.Extension -match '^\.(pdb|ilk|obj|tif|tiff|vtk|vti|vtp|vtr)$' }).Count) {
    throw 'Debug files or research datasets were found in the runtime payload.'
}
$builtExecutable = Join-Path $repositoryRoot 'x64\Release\OCTview3R.exe'
if (-not (Test-Path -LiteralPath $builtExecutable)) { throw 'Build Release before packaging.' }
if ((Get-FileHash $builtExecutable).Hash -ne (Get-FileHash (Join-Path $StandaloneDirectory 'OCTview3R.exe')).Hash) {
    throw 'The standalone EXE differs from the current Release build. Refresh deployment first.'
}

New-Item -ItemType Directory -Path $OutputDirectory | Out-Null
$payloadDirectory = Join-Path $OutputDirectory 'payload\OCTview3R'
New-Item -ItemType Directory -Path $payloadDirectory -Force | Out-Null
Get-ChildItem -LiteralPath $StandaloneDirectory -Force |
    Copy-Item -Destination $payloadDirectory -Recurse
# Refresh documentation without changing the original standalone folder.
foreach ($name in @('README.md', 'AUTHORS.md', 'CITATION.cff', 'THIRD_PARTY_NOTICES.md')) {
    Copy-Item -LiteralPath (Join-Path $repositoryRoot $name) -Destination $payloadDirectory -Force
}
Copy-Item -LiteralPath (Join-Path $repositoryRoot 'packaging\START_HERE.txt') -Destination $payloadDirectory -Force
Copy-Item -LiteralPath (Join-Path $repositoryRoot 'packaging\INSTALLER_README.txt') -Destination (Join-Path $payloadDirectory 'INSTALLATION.txt')
$docsDirectory = Join-Path $payloadDirectory 'docs'
New-Item -ItemType Directory -Path $docsDirectory -Force | Out-Null
foreach ($name in @('architecture.md', 'developer-guide.md', 'public-release-checklist.md', 'history-cleanup.md', 'release-test.md')) {
    Copy-Item -LiteralPath (Join-Path $repositoryRoot "docs\$name") -Destination $docsDirectory
}

# Ship the actual Qt runtime license texts, not only installer-distribution notices.
$qtSourceDirectory = [IO.Path]::GetFullPath((Join-Path $env:QTDIR '..\Src'))
$qtLicenseDirectory = Join-Path $payloadDirectory 'licenses\qt-runtime'
New-Item -ItemType Directory -Path $qtLicenseDirectory -Force | Out-Null
foreach ($name in @('LICENSE.GPLv3', 'LICENSE.LGPLv3', 'LICENSE.LGPLv21')) {
    $licensePath = Join-Path $qtSourceDirectory $name
    if (-not (Test-Path -LiteralPath $licensePath)) { throw "Missing Qt runtime license: $licensePath" }
    Copy-Item -LiteralPath $licensePath -Destination $qtLicenseDirectory
}
foreach ($module in @('qtbase', 'qtsvg')) {
    $thirdPartyDirectory = Join-Path $qtSourceDirectory "$module\src\3rdparty"
    if (-not (Test-Path -LiteralPath $thirdPartyDirectory)) { continue }
    Get-ChildItem -LiteralPath $thirdPartyDirectory -Recurse -File |
        Where-Object { $_.Name -match 'LICENSE|COPYING|NOTICE|COPYRIGHT|^FTL\.TXT$|^README' } |
        ForEach-Object {
            $relativePath = $_.FullName.Substring($qtSourceDirectory.Length).TrimStart('\')
            $destination = Join-Path $qtLicenseDirectory $relativePath
            New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
            Copy-Item -LiteralPath $_.FullName -Destination $destination
        }
}

$manifest = [ordered]@{
    application = 'OCTview3R'; version = $version; platform = 'windows-x64'
    source_commit = $commit; source_dirty = ($sourceState.Count -gt 0)
    test_package = [bool]$TestPackage; compiler = "Inno Setup $compilerVersion"
    built_utc = [DateTime]::UtcNow.ToString('o')
    files = @(Get-ChildItem -LiteralPath $payloadDirectory -Recurse -File | Sort-Object FullName |
        ForEach-Object { [ordered]@{
            path = $_.FullName.Substring($payloadDirectory.Length + 1).Replace('\', '/')
            bytes = $_.Length; sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        } })
}
$manifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $payloadDirectory 'BUILD-MANIFEST.json') -Encoding UTF8
$compilerArguments = @('--quiet', "--define=PayloadDir=$payloadDirectory", "--define=AppVersion=$version", "--define=BuildOutputDir=$OutputDirectory")
if ($TestPackage) {
    $compilerArguments += '--define=AppIdValue=OCTview3R.InstallerSmokeTest.x64'
    $compilerArguments += '--define=InstallerTestBuild=1'
}
& $ISCC @compilerArguments (Join-Path $repositoryRoot 'packaging\OCTview3R.iss')
if ($LASTEXITCODE -ne 0) { throw "Inno Setup failed: $LASTEXITCODE" }

$zipName = "OCTview3R-$version-windows-x64-portable.zip"
Add-Type -AssemblyName System.IO.Compression.FileSystem
[IO.Compression.ZipFile]::CreateFromDirectory($payloadDirectory, (Join-Path $OutputDirectory $zipName),
    [IO.Compression.CompressionLevel]::Optimal, $true)
$artifactNames = @("OCTview3R-$version-windows-x64-setup.exe", $zipName)
$artifactNames | ForEach-Object {
    $hash = (Get-FileHash -LiteralPath (Join-Path $OutputDirectory $_) -Algorithm SHA256).Hash.ToLowerInvariant()
    "$hash  $_"
} | Set-Content -LiteralPath (Join-Path $OutputDirectory 'SHA256SUMS.txt') -Encoding ASCII
Write-Host "Installer and portable ZIP created: $OutputDirectory"
if ($sourceState.Count -gt 0 -or $TestPackage) { Write-Warning 'LOCAL PREVIEW ONLY: dirty sources or test AppId. Do not publish these artifacts.' }
