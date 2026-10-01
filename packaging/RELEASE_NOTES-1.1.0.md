# OCTview3R 1.1.0

A self-contained Windows x64 release for viewing grayscale/RGB image volumes
together with segmented surfaces, meshes, and point data.

## Downloads

- **OCTview3R-1.1.0-windows-x64-setup.exe**: recommended installation for the
  current Windows user, with Start menu entry, optional desktop shortcut, and
  uninstall support. Administrator rights are normally not required.
- **OCTview3R-1.1.0-windows-x64-portable.zip**: extract the complete archive and
  run `OCTview3R.exe` in the `OCTview3R` folder; no installer is needed.
- **OCTview3R-1.1.0-source.zip**: corresponding source snapshot and build scripts.
- **SHA256SUMS.txt**: SHA-256 checksums for the downloads.

Both binary packages include Qt 5.15.2, VTK 8.2, Qt plugins, and the
application-local Microsoft Visual C++/OpenMP runtime. They contain no research
datasets and do not require a separate Qt, VTK, Visual Studio, or Python setup.
The source snapshot's ImageData directory contains only the documented
market-cherry TIFF/RAW examples and their README. Removed mesh exports, obsolete
data paths and former PDF/text references are excluded from the new snapshot.

## Requirements and installation notes

Windows 10/11 x64, an appropriate OpenGL graphics driver, and enough memory for
the datasets. Windows on ARM is not a supported target. Close OCTview3R before
updating. Uninstall does not deliberately remove personal data or preferences.

**The installer is unsigned.** Windows may display an unknown-publisher or
SmartScreen warning. Download only from this repository and verify the checksum.
Follow institutional IT policy; do not disable Windows security.

## Included improvements

- Grayscale and RGB volumes in a shared scene with PolyData and interactive
  slice planes, calibrated coordinate display, floating-point crop controls,
  and per-dataset transformations.
- Smooth scalar opacity, independent palette autoscaling and window/level,
  opaque-black slice planes, and memory checks for large RGB-to-grayscale changes.
- Compact dark-only interface, narrower default controls, OCT-volume branding,
  dataset information, and About dialog.
- Self-contained Qt Designer forms: palette, styling, icons, spacing, and layout
  constraints can be previewed and edited without an external stylesheet.
- Dataset colour changes preserve Designer styling; VTK teardown no longer
  requests extra renders or opens a diagnostic window when closing.
- Acknowledgement of Miroslav Zabic's advice on preparing the public release.
- Installer and pipeline regression tests, architecture documentation, updated
  screenshots, and an expanded JOSS manuscript draft.
- Updated README/paper interface illustration with grayscale volume and blue
  organoid geometry, explicitly described as independent demonstration datasets.
  The paper build also provides a review copy without the large DRAFT watermark;
  the manuscript remains unpublished.

## Verification and limitations

The production-pipeline/Qt harness performs 269 checks. A separate UI harness
performs 246 checks at each of 100%, 150%, and 200% scaling, including standalone
Designer forms, text fit, colour swatches, saved layouts, and VTK shutdown.
Packaging verification
covers installation, repeated installation, payload hashes, runtime tests with
developer dependency paths removed, and uninstall preserving an additional
synthetic user file. These local checks do not replace a clean-machine test,
rendered-image comparisons, scientific validation, or a performance benchmark.
The software is for research, not clinical decision-making.

An independent release test by Miroslav Zabic is planned but has not yet been
reported. The release remains a private draft pending author decisions and review.

## License and source

OCTview3R is GPL-3.0-only. Dependency licenses and source links are provided in
`THIRD_PARTY_NOTICES.md` and the package's `licenses` directory. The source
snapshot corresponds to the commit identified in `BUILD-MANIFEST.json`.
The software and paper are separate publication steps; no JOSS acceptance is
claimed. While the repository remains private, release access requires a GitHub
invitation.
