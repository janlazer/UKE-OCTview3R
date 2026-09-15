# Changelog

All notable changes to OCTview3R are documented in this file. The project
uses [Semantic Versioning](https://semver.org/).

## [1.1.0] - Unreleased

### Added

- Self-contained Windows x64 installer with per-user installation, Start menu
  and optional desktop shortcuts, uninstall support, portable ZIP, build
  manifests, and SHA-256 download checksums.
- Cyan OCT-volume logo, embedded Windows application icon, and branding in
  the main window and About dialog.
- PolyData range cropping with floating-point bounds.
- PolyData gloss control and improved initial lighting.
- Dataset metadata, loading progress, application themes, and an About dialog.
- Camera fit actions and improved object, plane, and transform interaction.
- Configuration-aware development and deployment scripts.
- Citation, license, third-party, and example-data documentation.
- Native RGB volume rendering with per-dataset grayscale conversion.

### Changed

- Restored smooth intensity-dependent opacity for grayscale volumes, with an
  optional uniform-opacity mode retaining the hard threshold appearance.
- Separated threshold-based palette autoscaling from automatic and manual
  window/level adjustments; threshold edits no longer overwrite window/level.
- Added synthetic pipeline and Qt-control regression tests for threshold,
  palette, window/level, opaque-black planes, and RGB behavior.
- Refreshed the dark theme with graphite panels, cyan accents, clearer
  control states, and visible spin-box and combo-box arrows.
- Refined the Qt Designer forms and reorganized the controls for clearer use.
- Synchronized planes with object rotation, translation, and scaling.
- Improved loading validation and error reporting for volume and PolyData input.
- Standardized the application version as 1.1.0.
- Corrected author and historical affiliation metadata.
- Corrected RGB TIFF luminance ranges and guarded large RGB-to-grayscale
  switches against excessive working-memory use.

### Removed

- Legacy VTK-family example data from the distributed repository.

[1.1.0]: https://github.com/janlazer/UKE-OCTview3R/releases/tag/v1.1.0
