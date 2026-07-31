# Example data

The TIFF volumes in this directory are optical coherence tomography scans of
a cherry. They are provided solely as non-medical demonstration data for
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

VTK-family examples (`.vtk`, `.vti`, `.vtp`, and `.vtr`) are deliberately
excluded from the published repository. Users may load their own compatible
data through the application's volume and PolyData import dialogs.
