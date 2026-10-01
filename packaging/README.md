# Windows release packaging

The installer wraps the existing `_standalone` runtime. End users do not need
Qt, VTK, Visual Studio, Python, or Internet access during installation.
Supported systems are Windows 10/11 x64 with a suitable graphics driver.

## Build

1. Build `Release | x64` and run `tests/run-threshold-regression.ps1`.
2. Create a fresh standalone deployment with
   `deploy-runtime.ps1 -Configuration Release -Standalone`. Use a fresh
   `-DestinationRoot` when an earlier package already exists; never mix builds.
3. Make sure the source tree is committed and obtain the official
   [Inno Setup 7.1+ compiler](https://jrsoftware.org/isdl.php).
4. Build from PowerShell (adapt the compiler location):

   ```powershell
   .\build-installer.ps1 -ISCC 'C:\Tools\Inno Setup 7\ISCC.exe'
   ```

The build requires the existing Qt source distribution at `QTDIR\..\Src`
for runtime license texts. It rejects missing runtime files, mismatched
standalone/Release executables, reparse points, and research/debug files.
It creates a separate payload and does not modify the original `_standalone`.
Use `-StandaloneDirectory` and `-OutputDirectory` to override the defaults.
Existing output directories must be empty; earlier releases are not overwritten.

Outputs under `dist/installer/`:

- `OCTview3R-<version>-windows-x64-setup.exe`
- `OCTview3R-<version>-windows-x64-portable.zip`
- `SHA256SUMS.txt`
- `payload/OCTview3R/`, including `BUILD-MANIFEST.json` with source commit,
  compiler version, and per-file hashes

The payload also includes the README's interface and About screenshots so its
image links work offline. These are illustrations, not loadable research data.

`-AllowDirty` permits local preview builds, not release publication.
`-TestPackage` changes the installer AppId to isolate smoke-test registration.
Never upload artifacts whose manifest has `source_dirty` or `test_package` set.
The packaging compiler is a build dependency, not a runtime dependency.

## Installer behaviour

Installation defaults to `%LOCALAPPDATA%\Programs\OCTview3R` for the current
Windows user. No elevation, global environment changes, or file associations
are required. English and German wizard translations are included. The
application's embedded icon is used for setup and shortcuts.

`OCTview3R.Desktop.x64` is the stable installer AppId for upgrades. Close the
viewer before updating. Only installer-owned files are removed on uninstall;
additional user files and existing application preferences are retained. Keep
datasets outside the installation directory. There is no recursive wildcard
cleanup of the install directory.

The current installer is unsigned. Do not represent it as a verified publisher
or recommend disabling SmartScreen. Signing requires an author-controlled
certificate or signing service and is a separate release-management decision.

## Verification and release

Test a fresh installation, a second installation over it, launch with developer
dependency paths removed, and uninstall. Use an isolated test AppId and directory
for local smoke tests. A local development-machine test does not replace a test
on a clean Windows PC or VM without Qt/VTK installed.

The automated harness installs twice, checks the payload hashes, runs the
production-pipeline/Qt regression executable beside the installed DLLs with
development paths removed, and verifies uninstall and preservation of an
additional synthetic user file:

```powershell
.\build-installer.ps1 -ISCC 'C:\Tools\Inno Setup 7\ISCC.exe' -TestPackage -OutputDirectory .\dist\installer-test
.\tests\test-installer.ps1 -PackageDirectory .\dist\installer-test
```

The test package uses a separate registration and does not create the normal
Start menu or desktop shortcuts. Tests do not launch the normal user session;
the regression harness uses separate INI preferences. Test logs and preserved
probe files remain in a unique directory under `tmp/installer-smoke-*`.

Before publication, verify that the packaged EXE matches the tested Release
build, the manifest is clean, the tag identifies that source commit, and the
checksums match. Create the GitHub release as a draft, upload the installer,
portable ZIP, checksums, and the matching source archive, then publish only when
all uploads succeed. Do not overwrite an existing published release's tag or
assets. Do not change repository visibility as part of release packaging.

Release notes should explain the two downloads, graphics/memory requirements,
unsigned-installer warning, source and dependency-license links, and tests run.
GitHub's automatic source archives do not contain runtime binaries. The Qt/VTK
source links and redistributed license notices are in `THIRD_PARTY_NOTICES.md`.
Recipients need repository access while it remains private.
