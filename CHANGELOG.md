# Changelog

All notable changes to OCTview3R are documented in this file. The project
uses [Semantic Versioning](https://semver.org/).

## [1.1.1] - Unreleased

### Fixed

- Initialize a neutral, disabled gradient-opacity table for VTK 8.2 so the
  RGBA volume shader cannot emit an unnamed sampler. Scalar opacity, thresholds,
  window/level, and the shared VTK installation remain unchanged.
- Pass RGB slice-plane colours through as three opaque channels instead of
  requesting an unsupported RGB-to-RGBA conversion without a lookup table.

### Added

- Real GPU rendering regression tests for scalar, RGB and RGBA volumes, all
  blend modes, colour-mode changes, transparency, and slice planes; optional
  local TIFF checks use the production reader without distributing the data.
- GPU regression checks against the DLLs from an isolated test installation.
- Ignore local root-level TIFF inputs while retaining the documented cherry
  examples. Prepare a new v1.1.1 tag without rewriting the existing v1.1.0 tag.

## [1.1.0] - Previous release candidate

### Added

- Self-contained Windows x64 installer with per-user installation, Start menu
  and optional desktop shortcuts, uninstall support, portable ZIP, build
  manifests, and SHA-256 download checksums.
- Cyan OCT-volume logo, embedded Windows application icon, and branding in
  the main window and About dialog.
- PolyData range cropping with floating-point bounds.
- PolyData gloss control and improved initial lighting.
- Dataset metadata, loading progress, a dark interface, and an About dialog.
- Camera fit actions and improved object, plane, and transform interaction.
- Configuration-aware development and deployment scripts.
- Citation, license, third-party, and example-data documentation.
- Native RGB volume rendering with per-dataset grayscale conversion.

### Changed

- Updated the shared README/paper screenshot to the compact interface with
  independently loaded grayscale volume and organoid geometry, and added a
  reproducible DRAFT-labelled manuscript review PDF. The literature comparison
  also covers Open Chrono-Morph Viewer and napari; the paper remains unsubmitted.
- Restored smooth intensity-dependent opacity for grayscale volumes, with an
  optional uniform-opacity mode retaining the hard threshold appearance.
- Separated threshold-based palette autoscaling from automatic and manual
  window/level adjustments; threshold edits no longer overwrite window/level.
- Added synthetic pipeline and Qt-control regression tests for threshold,
  palette, window/level, opaque-black planes, and RGB behavior.
- Refreshed the dark theme with graphite panels, cyan accents, clearer
  control states, and visible spin-box and combo-box arrows.
- Refined the Qt Designer forms and reorganized the controls for clearer use.
- Made dark mode the only interface theme and reduced control/toolbar spacing.
- Moved all static palette, styling, icons and sizing into the four `.ui` forms,
  with independent Designer previews and no delayed runtime dock-width override.
- Preserved Designer-defined swatch styling when dataset colours change.
- Added standalone-form and compact-UI checks at 100%, 150% and 200% scaling.
- Detached VTK widgets before closing the Qt render window to prevent a transient
  VTK diagnostic window and unwanted rendering during shutdown.
- Synchronized planes with object rotation, translation, and scaling.
- Improved loading validation and error reporting for volume and PolyData input.
- Standardized the application version as 1.1.0.
- Corrected author and historical affiliation metadata.
- Corrected RGB TIFF luminance ranges and guarded large RGB-to-grayscale
  switches against excessive working-memory use.

### Removed

- Legacy VTK-family example data from the distributed repository.

[1.1.0]: https://github.com/janlazer/UKE-OCTview3R/releases/tag/v1.1.0
[1.1.1]: https://github.com/janlazer/UKE-OCTview3R/releases/tag/v1.1.1
