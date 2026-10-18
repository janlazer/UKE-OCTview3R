# OCTview3R 1.1.1

Release candidate with a VTK 8.2 compatibility fix for RGB/RGBA volume rendering.
This release remains an **unpublished draft**; the repository is public.

## Fixes

- Prevent the invalid unnamed gradient-opacity sampler in generated VTK 8.2
  shaders, without changing scalar opacity, thresholds, window/level, or
  installing a modified shared VTK library.
- Correct the RGB slice-plane output format. Black on slice planes remains
  opaque, while rejected/black volume voxels remain transparent.
- Add real-GPU regression coverage and run it against installed runtime DLLs.

## Downloads

- **OCTview3R-1.1.1-windows-x64-setup.exe**: per-user installer with Start menu,
  optional desktop shortcut, update and uninstall support.
- **OCTview3R-1.1.1-windows-x64-portable.zip**: extract the complete archive and
  start `OCTview3R.exe` in the `OCTview3R` folder.
- **OCTview3R-1.1.1-source.zip**: source snapshot matching the binary manifest.
- **SHA256SUMS.txt**: SHA-256 checksums for all three packages.

The binaries include Qt 5.15.2, VTK 8.2, Qt plugins and the local Microsoft
Visual C++/OpenMP runtime. No separate development tools are required. Research
datasets are excluded; the source archive retains only the documented
market-cherry examples. The existing v1.1.0 Git tag is not rewritten.

## Requirements and limitations

Windows 10/11 x64, a suitable OpenGL graphics driver, and sufficient RAM/VRAM.
Close OCTview3R before updating. **The installer remains unsigned**: check the
checksum and follow institutional IT policy rather than disabling Windows
security. Draft assets require repository write access until publication.

Local verification covers 269 pipeline/Qt checks, 202 synthetic GPU checks,
and the compact-UI/shutdown harness. A local RGBA TIFF was additionally tested
through the production loader in RGB and grayscale, using MaxIP and composite.
Installation tests cover payload hashes, repeated installation, both regression
harnesses without development dependency paths, and uninstall preserving an
additional synthetic user file. These are development-machine checks, not a
clean-machine test, independent validation, or an image-quality benchmark.
An independent release test by Miroslav Zabic has not yet been reported.

OCTview3R is GPL-3.0-only; dependency notices and source links accompany the
packages. See `BUILD-MANIFEST.json` for the exact source commit. The JOSS paper
remains an unsubmitted draft. The software is for research, not clinical use.
