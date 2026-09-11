# OCTview3R visual identity

The cyan volume with scan planes and an amber segmented surface represents
the shared 3D space used to overlay OCT volumes and polygonal segmentations.

- `octview3r-logo.png`: selected original, 1254 x 1254 pixels, transparent RGBA.
- `octview3r-ui.png`: 512 px runtime resource for Qt and high-DPI displays.
- `octview3r.ico`: Windows icon containing 16, 20, 24, 32, 40, 48, 64,
  128, and 256 px images.

Rebuild the derived assets from the repository root with
`./tools/build-branding.ps1`. The script only resizes and packages the original;
it preserves transparency. The master PNG is not embedded in the executable.

The mark was created with Codex's built-in image-generation tool on
2026-09-11 and selected by Jan Hahn. No external logo or institution's mark
was used. These assets are distributed with the project under its GPL-3.0-only
license; see the repository LICENSE.

## Original generation prompt

Create a polished square application logo/icon for scientific 3D visualization
software named OCTview3R. The mark should visually combine an optical coherence
tomography volumetric scan and a segmented 3D surface: a compact translucent
isometric cube made of several subtle horizontal scan layers, with one elegant
organic curved mesh/contour shape visible inside and slightly emerging. Style:
modern scientific software, precise, trustworthy, distinctive, minimal
flat-vector appearance with restrained depth, crisp bold silhouette readable
at 16–32 px, no photorealism, no gradients that become muddy at small sizes,
no thin tiny details. Palette: charcoal/near-black structure, cool cyan/teal
as the main accent, one restrained warm amber highlight only if useful.
Transparent background. Centered with generous padding. Absolutely no text,
no letters, no watermark, no border, no mockup, no drop-shadow outside the
mark. Deliver as a high-resolution 1:1 raster suitable for downscaling to app icons.

## UI integration

Logo position and dimensions are defined in `OCTview3R.ui` and
`aboutdialog.ui`, so both placements are visible in Qt Designer.
The optional dark theme uses graphite surfaces with cyan accents. Scientific
data colours, axis colours, viewer background settings, and light/system themes
are independent of the branding. The header is outside the render widget,
so it is not included in exported scientific renderings.
