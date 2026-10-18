# Independent release test

Miroslav Zabic advised on preparing OCTview3R for publication. Jan Hahn confirmed
on 15 September 2026 that Miroslav intends to test the private release in the
following days. No test result has been reported yet.

## Record when the test is completed

- Tester: Miroslav Zabic
- Test date: pending
- Installer/source commit and SHA-256: record the exact package received
- Windows version, graphics card/driver, RAM: pending
- Installation without separately installed Qt/VTK: pending
- Load the 8-bit and 16-bit cherry TIFF stacks: pending
- Load a representative shareable PolyData file and check its OCT overlay: pending
- Check thresholds, window/level, crop, plane movement and object transforms: pending
- Close/reopen the viewer and uninstall: pending
- Problems and reproducible steps: pending

The 269 automated production-pipeline/Qt checks are a separate local test.
Update the acknowledgement to include release testing only after a real result
has been received. Do not infer a passed test from sending the installer.

## Local packaging checks - 1 October 2026

The current Designer-driven dark interface was checked locally before refreshing
the private v1.1.0 draft. Release and Debug x64 builds succeeded. The compact-UI
harness passed 246 checks at each of 100%, 150%, and 200% scaling, and the separate
production-pipeline/Qt harness passed all 269 checks.

An isolated test installer passed installation, repeated installation, all 249
payload-file hashes, the 269 runtime checks with developer dependency paths
removed, and uninstall while preserving an additional synthetic user file.
The test AppId and preferences were separate from the normal viewer installation.

These are development-machine checks, not Miroslav's independent test, a
clean-machine validation, or a scientific image-quality assessment. The installer
remains unsigned. Use the refreshed package's `BUILD-MANIFEST.json` and
`SHA256SUMS.txt` to identify the exact source commit and binaries.

## Local GPU and packaging checks - 18 October 2026

Release and Debug x64 builds succeeded for the v1.1.1 candidate. The fix registers
a neutral, disabled gradient-opacity table for VTK 8.2 and corrects RGB slice
output component counts. The tests passed:

- 269 production-pipeline/Qt checks.
- 202 real-GPU synthetic checks covering scalar/RGB/RGBA, all four blend modes,
  repeated colour-mode changes, rejected thresholds, zero opacity, and planes.
- 228 GPU checks when additionally loading a locally supplied RGBA TIFF through
  the production reader and rendering RGB/grayscale in MaxIP and composite.
  That research input remains local and is excluded from Git and release assets.
- 246 compact-UI/shutdown checks at each of 100%, 150%, and 200% scaling.
- Isolated test installation and repeated installation with 251 payload hashes,
  followed by the 269 pipeline checks and 202 GPU checks against installed DLLs
  with development dependency paths removed. Uninstall preserved an additional
  synthetic user file. The normal user installation was not modified.

The GPU harness checks generated-shader errors and basic rendered visibility,
not pixel-identical rendering across vendors or scientific image fidelity.
The test installation is separate from the final normal-AppId installer;
its test package must never be uploaded. These remain development-machine tests,
not Miroslav's pending independent test. The v1.1.1 GitHub release stays a draft,
the installer remains unsigned, and the existing public v1.1.0 tag is preserved.
