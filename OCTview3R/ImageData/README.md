# Example data

The TIFF volumes in this directory are optical coherence tomography scans of
a commercially available cherry purchased at a market, as confirmed by Jan Hahn.
They are provided solely as non-medical demonstration data for
OCTview3R and contain no human, patient, or animal subject information.

Included TIFF files:

- `100x100x100_8bit.tif`
- `_70x100x100_8bit.tif`
- `Auswahl/100x100x100_8bit.tif`
- `Auswahl/100x100x100_16bit.tif`

The files differ in bit depth and/or selected slice range and can be used to
exercise volume loading, thresholds, cropping, planes, and rendering.

The cherry TIFF files were created for OCTview3R and are distributed under
the repository's GPL-3.0-only license.

VTK-family examples (`.vtk`, `.vti`, `.vtp`, and `.vtr`) were removed from the
current checkout and are ignored for new additions. They still exist in earlier
Git commits; removing a file from the working tree does not remove its history.

The cherry statement applies to the listed TIFF examples, not automatically to
all other files in this directory. Legacy RAW, BYU, STL, PLY, XYZ, TXT, C++ example
and PDF files are still tracked. Their individual provenance and redistribution
terms have not all been documented. In particular, do not infer the origin of
`lens3D_polydata_*.ply` or permission to redistribute the PDFs from the TIFF
provenance. See [public-release checks](../../docs/public-release-checklist.md).

Users may load their own compatible data through the volume and PolyData dialogs.
