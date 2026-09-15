# OCTview3R architecture and rendering conventions

This technical companion to the [JOSS manuscript](../paper/paper.md) describes
the current 1.1.0 development state, checked on 14 September 2026. It documents
implementation choices rather than a new rendering algorithm. The scientific
background and related applications are cited in the manuscript's
[bibliography](../paper/paper.bib).

For a source-reading order, ownership contracts, UI invalidation table and an
extension checklist, see the [developer guide](developer-guide.md).

## Components and ownership

| Component | Responsibility | Main source |
| --- | --- | --- |
| Main window and Designer forms | Input controls, active-dataset selection, units, and UI synchronization | [OCTview3R.cpp](../OCTview3R/OCTview3R.cpp), [OCTview3R.ui](../OCTview3R/OCTview3R.ui) |
| `DocumentModel` | Own dataset state and manage document lifetime | [documentModel.cpp](../OCTview3R/documentModel.cpp) |
| `ImageData` | Store one dataset's input, VTK resources, settings, and update flags | [viewerData.h](../OCTview3R/viewerData.h) |
| `ViewerController` | Coordinate one renderer, camera, illumination, and scene decorations | [viewerController.cpp](../OCTview3R/viewerController.cpp) |
| `VolumePipeline` | Crop, threshold, colour/opacity mapping, RGB conversion, and slice planes | [volumePipeline.cpp](../OCTview3R/volumePipeline.cpp) |
| `PolyPipeline` | Clip and display geometry with normals and material properties | [polyPipeline.cpp](../OCTview3R/polyPipeline.cpp) |
| Loading worker | Read files and calibration; report progress, results, and errors | [loading.cpp](../OCTview3R/loading.cpp) |

`DocumentModel` owns dataset state through `std::unique_ptr`; VTK resources in
that state use `vtkSmartPointer`. An active tab selects which state the controls
edit; it does not create a separate rendering space. Switching tabs must restore
that dataset's controls without applying another dataset's settings.

Dirty flags distinguish filtering, appearance, transform, plane, and visibility
changes. For example, moving an actor changes its transform, not its source
voxels. A crop changes filter inputs; RGB brightness changes require updating
derived colour data. This separation avoids unnecessary pipeline reconstruction,
but does not imply that every interaction is constant-time or allocation-free.

File loading runs in a worker thread. Reader progress is forwarded where the
reader supports it; an indeterminate indicator is used otherwise. Results return
to the main thread for dataset insertion and scene/UI updates.

## Two data paths, one scene

```text
Volume input -> vtkExtractVOI -> scalar or RGB display branch -> vtkVolume
                           \-> image mapping -> associated slice plane

Geometry input -> optional vtkBox clipping -> normals -> vtkPolyDataMapper
                                                    -> vtkActor

Dataset-specific transforms -> one vtkRenderer -> shared camera and viewport
```

The implementation uses `vtkSmartVolumeMapper`, not a custom ray caster.
Available volume blend modes include composite, additive, minimum intensity,
and maximum intensity. RGB datasets initially use composite rendering. An
intensity projection is a different visualization operation and should not be
interpreted as a colour-preserving view of every voxel.

### Scalar volumes: separate visibility and appearance

The cropped image passes through `vtkImageThreshold`. Accepted voxels retain
their source scalar values; rejected voxels become zero in the derived output.
The source image and file are not overwritten. Colour and opacity transfer
functions then determine the rendered appearance.

Four settings are deliberately distinct:

| Setting | Meaning |
| --- | --- |
| Min/Max threshold | Accepted source-intensity interval; values outside it are transparent in the volume |
| Smooth opacity | Linear opacity ramp across the accepted interval; disabling it gives uniform opacity within the interval |
| Palette autoscale | Map the accepted interval across the palette when enabled, or retain the full source-range mapping when disabled |
| Auto window / manual window and level | Neutral full-source-range brightness mapping, or an independently chosen contrast span and centre |

For a non-degenerate threshold interval `[L, U]`, smooth opacity increases from
zero at `L` to the dataset opacity at `U`. Zero is always transparent, including
when signed source data straddle zero. A single accepted nonzero intensity uses
uniform opacity. The ramp changes per-sample opacity, not an anatomical boundary;
compositing through multiple samples also affects the final appearance.

Changing thresholds does not silently rewrite manual window/level. Editing
window/level disables automatic windowing, but does not disable palette
autoscaling. Settings are stored separately for each dataset. False colour,
interpolation, and opacity can emphasize or suppress structures; they are not
substitutes for inspecting the original intensity slices.

### RGB volumes and slice planes

The RGB branch extracts the first three components. `vtkImageLuminance` supplies
the intensity used to threshold a binary alpha mask. A common window/level
mapping adjusts all three colour channels, which are assembled with that mask
into an RGBA image for dependent-component rendering. Input alpha from a fourth
component is replaced, not preserved. Smooth scalar-opacity ramping does not
apply to this RGB mask; the interface disables that control in RGB mode.

The plane branch intentionally differs from volume transparency. Its mapped
image retains opaque black pixels, including black background/rejected pixels,
so the plane remains a continuous slice. Optional median filtering belongs to
the plane branch; it is not a claim that the source volume has been denoised.

Each plane uses the transform of its parent volume. The
[transformable plane widget](../OCTview3R/transformableImagePlaneWidget.cpp)
keeps plane rendering and interaction in the corresponding object space.

### PolyData and occlusion

Geometry supports points, wireframes, and surfaces. Cropping uses a `vtkBox`
with `vtkClipPolyData`; surface geometry passes through normal generation.
The mapper uses the selected object colour rather than assuming that arbitrary
input scalars encode a segmentation label. Material settings expose opacity and
specular reflection (gloss).

The shared renderer uses camera-following illumination and requests depth
peeling, including volumes, with alpha bit planes enabled and multisampling
disabled. PolyData actors use the translucent rendering path even at full
opacity to avoid changing rendering passes at the opacity endpoint. The intended
relationship is spatial occlusion, not active-tab priority. Correctness across
all OpenGL drivers, blend modes, and transparency combinations has not been
established by automated image comparisons.

## Coordinates, calibration, and cropping

The TIFF reader handles supported X/Y resolution tags, their units, and spacing
information from the image description. Recognized physical calibration is
converted to millimetres internally and tracked per axis. Volume controls
present calibrated distances in micrometres; unknown calibration is not
silently replaced by assumed millimetres. TIFF volume origin is set to
`(0, 0, 0)`, rather than the geometric centre.

For a calibrated axis, displayed micrometres are converted back to millimetres
before calculating an index from `(coordinate - origin) / spacing`. Crop
controls round to the nearest voxel and clamp to valid bounds. The upper crop
bound is exclusive in the UI; `vtkExtractVOI` receives the corresponding
inclusive upper index. Consequently, an `N`-voxel crop interval and the centre
of the last voxel, at index `N - 1`, are different concepts. Geometry cropping
uses floating-point data coordinates without voxel rounding.

Each dataset owns a separate transform, applied as scale, X/Y/Z rotation, and
translation. Moving a volume moves its associated plane, but does not move an
independently imported mesh. A shared renderer does not establish registration
or convert mesh units. For example, geometry exported in micrometres must be
converted or scaled to match a volume stored internally in millimetres. The
viewer performs neither automatic registration nor refractive-index correction.

For an overlay inspection, verify units, origin, axis orientation, and landmark
alignment before interpreting apparent surface agreement. After independently
changing a dataset transform, repeat that check. A visually plausible overlay
alone is not a quantitative estimate of segmentation or registration accuracy.

## Memory and deployment

Processing is in-memory. RGB-to-grayscale conversion can require additional
working buffers even when the final grayscale output is smaller. On Windows,
the interface estimates the cropped working set, considers reclaimable derived
RGB buffers, and retains a safety reserve against available physical memory. It
can reject a conversion before allocating an excessive working set. This guard
is not an out-of-core implementation or a guarantee against every allocation
failure; available memory can change concurrently.

The current supported build is Windows x64 with Qt 5.15.2 and VTK 8.2. The
[deployment script](../deploy-runtime.ps1) packages the executable, matching
libraries, Qt plugins, runtimes, and license notices. Graphics-driver support
is still required. Application settings retain selected UI preferences, not a
complete reproducible scene containing every dataset and parameter.

## Verification boundaries

Run the existing production-pipeline and Qt-control regression harness with
the repository's configured build environment:

```powershell
.\tests\run-threshold-regression.ps1
```

The [test implementation](../tests/threshold-regression.cpp) uses synthetic
8-bit, 16-bit, and RGB inputs and currently performs 269 checks. It exercises
opacity/colour functions, intensity preservation, threshold limits, plane
black-pixel alpha, colour-mode switching, and independent per-dataset controls.
Separate test settings avoid changing user preferences. Logs and control
snapshots are written under `tmp/threshold-regression/`.

This coverage does not establish clinical validity, segmentation accuracy,
cross-platform equivalence, maximum dataset capacity, or rendering speed.
Rendered-volume image baselines, broader import/calibration tests, memory-stress
tests, and continuous integration remain useful next steps. Full scene
serialization would also improve reproducibility of published views.
