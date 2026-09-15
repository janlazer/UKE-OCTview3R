# Example data

The retained image data are OCT scans of an ordinary cherry purchased at a
market. Jan Hahn confirmed their origin and the selection below on 15 September
2026. They are non-medical demonstration data and contain no human, patient or
animal subject information.

| File | Format | Dimensions |
| --- | --- | --- |
| `100x100x100_8bit.tif` | 8-bit TIFF stack | 100 x 100 x 100 |
| `100x100x100_16bit.tif` | 16-bit TIFF stack | 100 x 100 x 100 |
| `100x100x100_8bit.raw` | Unsigned 8-bit, headerless RAW | 100 x 100 x 100 |

For RAW import, enter the dimensions explicitly; the RAW file does not encode
them or a physical voxel spacing. The examples can exercise loading, thresholds,
cropping, planes and rendering. They are distributed under GPL-3.0-only.

Removed VTK-family exports and obsolete example-data paths are excluded from
the rewritten branch/tag history. Their private local backup is not part of
the repository or release packages. The application still accepts users' own
VTK, VTI, VTP and VTR inputs.

## Reference files still awaiting a decision

`A03-R-039.pdf`, `ReadAllPolyDataDemo.pdf`, `file-formats.pdf` and
`Example_CutMesh.txt` remain reference material, not cherry data. Their
redistribution review is still open; do not infer permission from the image-data
provenance. See [public-release checks](../../docs/public-release-checklist.md).
