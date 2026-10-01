# Developer guide

This guide describes the current Qt 5.15.2 / VTK 8.2 Windows implementation.
Start with [architecture.md](architecture.md) for rendering semantics and known
limitations, and the root [README](../README.md) for toolchain setup.
Header comments use Doxygen-compatible syntax; generated API HTML is not yet
part of the build. They describe contracts, not a stable external plugin API.

## Suggested reading order

1. `main.cpp` and `OCTview3R.ui`: application setup and the Designer-owned layout.
2. `viewerData.h`: per-document resources, units, appearance and dirty flags.
3. `documentModel.h/.cpp`: ownership and active-document selection.
4. `viewerController.h/.cpp`: one renderer, camera, decorations and dispatch.
5. `volumePipeline.h/.cpp` and `polyPipeline.h/.cpp`: actual filtering/rendering.
6. `OCTview3R.h/.cpp`: editor synchronization, user actions and worker lifecycle.
7. `loading.h/.cpp`: import, calibration, reference transfer and reader progress.
8. `transformableImagePlaneWidget.h/.cpp`: local/world interaction conversions.

## Ownership and thread boundaries

`DocumentModel::create()` allocates the VTK resources and immediately owns the
new `ImageData`. Its returned reference and the window's `activeImageData`
pointer are borrowed. Adding a document preserves existing object addresses;
removing one may destroy it. Do not cache a raw pointer beyond that lifetime.

Closing a tab calls `takeAt()`, which transfers ownership into a local
`unique_ptr`. While that owner is still alive, `ViewerController::detach()`
removes the interaction observer, disables/disconnects the plane widget and
removes volume/geometry props. The UI then selects another document before the
local owner is destroyed. `ViewerController::clear()` renders the emptied scene
and does not itself erase model documents. Window teardown instead calls the
idempotent `shutdown(documents)` before Qt hides its OpenGL widget, with a fallback
in the destructor. It blocks indirect widget renders, detaches all documents and
decorations, and removes the borrowed window/interactor links before model deletion.
Do not call `Off()` on a never-enabled plane or orientation widget: VTK 8.2 reports
an unassigned interactor even for that disable request, opening its output window.

File-loading sequence:

```text
GUI: validate dialog -> construct worker (copy parameters) -> moveToThread
worker: read/calibrate -> Register one transfer reference -> emit result
GUI: adopt result into vtkSmartPointer -> Delete transfer reference -> add tab
worker: emit finished -> deferred worker deletion and thread shutdown
GUI: initialize selected pipeline -> update decorations -> render
```

The result signal is queued across threads. Exactly one receiver owns its extra
VTK reference, including the rejected-result path. Connecting another consuming
receiver without revising the contract can double-release the result; an
additional observer should only borrow it. Ordinary reference-counted pipeline
members must not be manually deleted.

Only the GUI thread mutates documents, widgets, camera and renderer. The worker
copies dialog values before it starts and does not retain the dialog pointer.
Its progress observer uses a stack context valid only during synchronous
`vtkAlgorithm::Update()`. Not every reader emits intermediate progress. Thread
interruption/quit requests do not cancel a running synchronous VTK reader;
the window destructor waits for readers to finish.

## UI updates and invalidation

The normal path is control signal -> guarded slot -> document fields -> dirty
flags -> `refreshViewer()` -> selected pipeline -> decorations -> render.
Each `mark*Dirty()` helper sets only one flag; it does not render, infer another
flag, or reconnect plane input. Pipelines clear the flags after applying them.

The existing handlers use these combinations:

| Change | Dirty flags / additional state | Existing entry point |
| --- | --- | --- |
| Rotation, shift, scale | `transformDirty` | `slotRotX`, `slotShiftX`, `slotScaleX` and Y/Z equivalents |
| Threshold interval | `appearanceDirty`, `dataPipelineDirty` | `slotSetThreshold` |
| Palette, scalar smooth opacity, object opacity, mesh gloss | `appearanceDirty` | `slotSetColormap`, `slotSmoothOpacity`, `slotSetObjectOpacity`, `slotSetPolyGloss` |
| Window/level or auto-window | `appearanceDirty`; also `dataPipelineDirty` in RGB mode | `slotSetWindowWidth`, `slotSetWindowLevel`, `slotAutoWindow` |
| RGB/grayscale switch | Appearance, data and plane dirty; `changePlaneInput = true` | `slotSetColorMode` |
| Plane median/orientation | `planeDirty`, `changePlaneInput = true`; orientation also sets `orientChanged` | `slotMedianCheckBox`, `slotOrientationChanged` |
| Existing plane drag or clipping-side flip | `planeDirty`; preserve the existing input/placement | `onePlaneCallbackFunction`, `slotCheckFlipPlane` |
| Object visibility | `visibilityDirty` | `slotShowObject` |
| Tab selection or scene decorations | No dataset invalidation; `refreshDecorations()` | `slotSetImageData(int)`, decoration slots |

Cropping is deliberately deferred: editing a range marks the controls pending;
`slotApplyRanges()` validates and commits all six limits, marks the data pipeline,
and updates plane configuration as needed. Do not run a full crop on each digit
typed. Merely setting `planeDirty` does not reconnect the plane input:
`VolumePipeline::updatePlane()` gates that work with `changePlaneInput`.

`slotSetImageData(ImageData*)` blocks child-object signals while restoring a
dataset's editor values and enabled states. Preserve this boundary when adding a
control, and restore its value here. Otherwise selecting another tab can invoke
slots with a mixture of old and new settings. Camera reset is an explicit,
one-shot first-load request, not an ordinary consequence of a render refresh.

The legacy name `onePlaneCallbackMutex` denotes a GUI-thread reentrancy guard,
not a lock. The callback currently refreshes the active document; it does not
map its `caller` back to an arbitrary document. If extending multi-plane editing,
review that routing explicitly rather than assuming all planes are selected.

## Coordinates and plane interaction

- Volume `VOI` and `sourceVOI` are half-open voxel-index pairs, in X/Y/Z order.
  A 100-voxel axis is `[0, 100)`. `vtkExtractVOI` needs `[0, 99]`; conversion is
  at the pipeline boundary. The upper boundary is not the last voxel centre.
- For a calibrated axis, the editor shows `1000 * (origin + index * spacing)`
  in micrometres. Applying a crop reverses this conversion and rounds to an index.
  Do not store GUI micrometres in `VOI`. Uncalibrated axes retain index controls.
- `spacingInMillimetresMask` bits 0/1/2 identify known X/Y/Z physical calibration.
  Unknown physical units must not be labelled as millimetres by inference.
- PolyData bounds stay floating point in native input coordinates. They are
  neither voxel-rounded nor automatically registered/unit-converted to a volume.
- Dataset transforms apply scale, X/Y/Z rotations in degrees, then translation.
  Translation uses scene units; it is not automatically rescaled to GUI micrometres.
  Each imported dataset has its own transform.
- Plane corners/reslice geometry are local image coordinates, including spacing.
  Display actors inherit the volume transform; mouse picks return in world space
  and must be inverse-transformed for widget motion and image sampling. Non-null
  transforms must be invertible. Transform normals with normal-transform rules,
  not the point transform, to handle anisotropic scaling.
- Volume mapper clipping uses a world-space plane derived from the local widget.
  `showPlane` enables clipping/interaction; `planeIsVisible` only hides its texture
  and margins. Black plane pixels stay opaque even when black volume voxels do not.

## Editing the GUI in Qt Designer

The four `.ui` forms (`OCTview3R.ui`, `aboutdialog.ui`, `opendata.ui` and
`openpoly.ui`) own **all static appearance**: layout, margins, sizes, icons,
initial states, the dark palette and the compact stylesheet. Open the relevant
form directly in Qt Designer and use **Form > Preview** (Ctrl+R). No external
stylesheet or forced Fusion style is needed. Keep `OCTview3R.qrc` alongside the
forms so Designer can resolve the logo, toolbar icons and SVG control arrows.
The VTK widget is a placeholder in Designer unless its Designer plugin is
installed; the live 3D scene is necessarily created by the application.

Select the top-level form in the Object Inspector to edit its `palette` and
`styleSheet` properties. Each form carries its own theme so that even the import
and About dialogs can be previewed independently; apply intended shared-theme
changes to all four forms. Individual widget styles remain editable locally.
Do not reintroduce a separate `.qss` file or hard-coded styling in C++.

Dock constraints, size policies and the preferred space for the visualization
header (`viewerBrandSpacer.sizeHint`) determine the default split in the main
form. The running application still restores the user's saved window/dock layout;
Designer previews the form's defaults, not those personal settings. There is no
delayed C++ dock-width override.

Runtime code only connects behaviour and updates data-dependent content such as
dataset names, units/ranges, enabled states, progress and selected dataset colours.
Colour swatches retain their Designer styling when their data colour changes.
About version/library strings are also dynamic. The application copies its
palette, stylesheet and icon **from the main form** for built-in Qt dialogs and
tooltips, rather than maintaining a second theme definition in C++.

## Adding a feature safely

1. Place persistent per-dataset values in `ImageData`, or scene-wide values in
   `Settings`. Choose explicit units, ranges and defaults; explain non-obvious
   invariants next to the field.
2. Add static layout in the relevant `.ui` form, keeping Qt Designer useful.
   Connect dynamic behaviour in the relevant `setup*` helper. Account for both
   slider and spin-box input paths and restore state on tab selection.
3. Validate volume/mesh applicability and handle rejected input before changing
   document state. A colour-mode change must preserve the memory guard.
4. Select the appropriate invalidation flags from the actual consumer code.
   Keep processing in the focused pipelines rather than growing GUI slots.
5. Add synthetic tests for both the new behaviour and adjacent invariants:
   transparency, plane black values, dataset isolation and coordinate boundaries.
6. Update architecture/user documentation when behaviour changes. Document why a
   VTK workaround exists, including the affected version; do not preserve dead
   implementations as commented-out code.

## Build and verify

Use the configured Qt/VTK paths from the root README. In a Visual Studio developer
PowerShell, build with `MSBuild OCTview3R.sln /p:Configuration=Release /p:Platform=x64`.
Then run:

```powershell
.\tests\run-threshold-regression.ps1
.\tests\run-compact-ui-regression.ps1 -Scale 1
.\tests\run-compact-ui-regression.ps1 -Scale 1.5
.\tests\run-compact-ui-regression.ps1 -Scale 2
python -B tests/check-paper.py
python -B paper/render_review_pdf.py
```

The threshold harness exercises production pipelines and Qt controls with separate
test preferences. Its 269 checks are not an image-quality, performance or clinical
validation. The compact-UI harness first constructs the four generated forms
without the application constructor, theme adapter or saved settings, verifying
that Designer previews are self-contained. It separately checks settings migration,
control sizes and text fit at 100%, 150% and 200% scaling, including a small window
and import/About dialogs. It also checks render-free, diagnostic-free teardown
for empty scenes, PolyData, volumes, and volumes with active planes/annotations.
It writes review screenshots below
`tmp/compact-ui-regression/scale-<factor>/` without touching user preferences.
Keep layout dimensions and control padding in the `.ui` forms (including their
`styleSheet` properties); reducing font sizes is not the intended compaction mechanism.

The Python renderer creates a local review PDF, not an official JOSS
proof; the official paper workflow must be run against the intended pushed revision.
Neither paper checks nor a PDF build constitute Windows application CI.

For installer changes, follow [packaging/README.md](../packaging/README.md).
Use a fresh output directory and `-TestPackage` for automated installation tests;
never run those tests against a user's ordinary installation.
