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
