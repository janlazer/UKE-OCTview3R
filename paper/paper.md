---
title: "OCTview3R: Interactive visualization of volumetric OCT and segmented three-dimensional data"
tags:
  - C++
  - Qt
  - VTK
  - optical coherence tomography
  - volume rendering
  - scientific visualization
authors:
  - name: Jan Hahn
    orcid: 0000-0003-3416-636X
    affiliation: "1, 2"
  - name: Giovanno Möbes
    affiliation: "2"
  - name: Tammo Ripken
    affiliation: "2"
affiliations:
  - index: 1
    name: University Medical Center Hamburg-Eppendorf (UKE), Hamburg, Germany
  - index: 2
    name: Laser Zentrum Hannover e.V. (LZH), Hannover, Germany
date: 2 October 2026
bibliography: paper.bib
---

# Summary

Optical coherence tomography (OCT) provides micrometre-scale cross-sectional
images of scattering samples [@huang1991]. Three-dimensional studies produce
both intensity volumes and derived geometry, including segmented points and
surfaces. Viewing geometry without the source signal can conceal misplaced
boundaries or registration offsets.

`OCTview3R` is a C++/Qt Windows application for interactive, offline inspection
of these complementary representations. Dedicated pipelines built with the
Visualization Toolkit (VTK) place scalar or RGB image stacks and independently
generated geometry in one spatial scene [@schroeder1996; @vtkbook2006].
Per-dataset appearance controls, cropping, transforms, and an interactive slice
plane support segmentation quality control and figure preparation. The
contribution is a focused research interface and maintainable integration of
established visualization methods, not a new reconstruction, segmentation, or
registration algorithm.

![OCTview3R 1.1.0 with its compact dark interface, displaying a grayscale volume
together with separately imported PolyData: red surfaces and blue meshes.
Colours distinguish displayed geometry rather than quantitative measurements.
The control panel provides per-dataset appearance, opacity, gloss, transforms,
and crop ranges. This interface illustration does not establish segmentation
or registration accuracy.](figures/octview3r-interface.png){ width=100% }

# Statement of need

Researchers exchanging OCT volumes and segmentation results through files need
to check whether derived structures coincide with the measured signal. A
coordinate list may be numerically valid yet mirrored, truncated, or displaced.
Joint volume/surface inspection provides context that neither isolated slices
nor surface-only views provide readily. OCT speckle also carries structural
information while affecting image interpretation [@schmitt1999]; changing a
display threshold must therefore not be confused with establishing a biological
boundary.

The intended users are researchers and engineers inspecting stored image
stacks, fitted surfaces, and segmented regions. RAW, TIFF, JPEG, and VTK-family
image inputs can be combined with geometry formats including VTK PolyData,
VTP, STL, PLY, OBJ, and XYZ. Direct controls expose volume blending, geometric
representation, opacity, colour, and spatial alignment without requiring users
to assemble a visualization pipeline. The practical objective is traceable
visual comparison, not automated segmentation validation. Consistent input
coordinates and independent quantitative checks remain the user's
responsibility.

# State of the field

Direct volume rendering separates intensity-dependent appearance and opacity
from explicit surface extraction [@levoy1988]. Combining volumes with polygonal
geometry likewise has a long history [@levoy1990]. `OCTview3R` applies these
established ideas through VTK rather than introducing a competing renderer.

Among open-source applications, `OCTproZ` addresses live OCT acquisition, GPU
signal processing, and visualization through an extensible plug-in architecture
[@zabic2020]. ParaView provides broad scientific-visualization workflows
[@ahrens2005]; 3D Slicer supports medical-image computing [@fedorov2012];
ITK-SNAP emphasizes anatomical segmentation [@yushkevich2006]; and Fiji provides
an extensible platform for biological-image analysis [@schindelin2012]. These
systems offer substantially broader acquisition or analysis capabilities.

`napari` provides multidimensional image viewing in Python, combining image,
label, point, and surface layers with programmatic control and a plug-in
ecosystem [@napari2019]. Open Chrono-Morph Viewer uses Qt and VTK to inspect
heterogeneous volumetric time series, with dynamic clipping surfaces and
scriptable animations [@faubert2025]. These applications demonstrate existing
support for layered visualization and interactive volume exploration.
`OCTview3R` instead concentrates on file-based OCT/segmentation inspection in a
dedicated desktop interface, rather than a general Python analysis environment
or a temporal animation workflow.

ParaView and 3D Slicer also support extensions, including additions to application
functionality [@paraviewplugins; @slicerextensions]. `OCTview3R` does not address an
absence of extensibility in these platforms. It deliberately targets a narrower
workflow: joint inspection of processed OCT volumes and externally generated
segmentation geometry. Its dedicated interface combines volume appearance,
object transformations, and associated slice-plane interaction; each plane
follows its parent volume's transformation.

An independent Qt/VTK application gives the research team direct control over
interface and pipeline changes without requiring integration into a host
application's extension architecture. This build-versus-contribute choice
prioritizes a focused inspection workflow and local source-level customization,
while retaining responsibility for maintaining the application and dependencies.
The trade-off is reduced scope: there is no scripting interface, automated
registration, distributed processing, or embedded segmentation. The viewer
complements these platforms; no comparative stability, performance, or usability
advantage is claimed.

# Software design

## Ownership and update flow

`DocumentModel` owns an `ImageData` state object for each dataset, including VTK
smart pointers, appearance settings, and spatial bounds. `ViewerController`
coordinates the renderer and delegates processing to `VolumePipeline` and
`PolyPipeline`. Dirty flags distinguish data filtering, appearance, transforms,
plane updates, and visibility, so an appearance change need not reconstruct the
entire pipeline. Self-contained Qt Designer forms define the static interface,
including the compact dark styling, palette, icons, and layout constraints;
developers can preview and edit the interface without a separate runtime theme.
Runtime code connects behaviour and updates dataset-dependent controls.
Background loading reports reader progress when available and returns results
to the main thread for scene updates.

## Intensity, colour, and opacity

The scalar pipeline extracts a volume of interest, thresholds source values,
and uses colour and opacity transfer functions for rendering. Transfer-function
design determines which structures become visible [@kindlmann1998]. Accordingly,
threshold limits, palette autoscaling, window/level, and opacity are separate
controls. The default smooth opacity increases across the accepted intensity
interval; a uniform-opacity alternative retains a hard threshold. Zero-valued
voxels remain transparent. Palette autoscaling maps the selected interval onto
the colour map, while automatic windowing supplies a neutral full-source-range
brightness mapping. Manual window/level changes do not redefine the threshold.
These are display operations, not source-data edits.

For RGB input, the first three components supply colour and luminance supplies
a binary threshold mask. A common window/level mapping adjusts all channels
before RGBA assembly; existing input alpha is not preserved. Scalar and RGB
slice planes retain opaque black pixels, unlike the transparent background of
the volume. Composite, additive, and intensity-projection modes are available.
False-colour palettes aid exploration but can introduce perceptual emphasis
[@crameri2020]; colours used to distinguish geometry in the interface screenshot
are display choices, not a quantitative colour scale.

## Coordinates and joint rendering

TIFF calibration is imported when supported resolution and spacing metadata
provide units. Calibrated spacing is stored in millimetres and displayed in
micrometres; uncalibrated axes are not assigned invented physical units. TIFF
volumes retain a zero origin instead of being centred. Physical crop limits
are rounded to voxel indices, whereas PolyData clipping uses floating-point
data coordinates.

Each dataset has its own translation, rotation, and anisotropic scaling. Its
associated slice plane follows that transform; independently imported geometry
does not automatically inherit another dataset's transform. Users must provide
compatible units and registration. PolyData supports points, wireframes, and
surfaces, with clipping, normals, and adjustable specular reflection. Volumes
and geometry share one renderer, which requests depth peeling including volumes
to support spatial occlusion rather than selection-based foreground ordering.
Transparency behaviour remains dependent on the VTK/OpenGL configuration.

![Separate volume and geometry pipelines feed one spatial scene. Each dataset
retains its own transform; an image plane follows its parent volume. Compatible
coordinates must be supplied by the analysis workflow.](figures/overlay-pipeline.png){ width=100% }

## Verification and limitations

A synthetic regression harness exercises the production pipelines and Qt
controls with 8-bit, 16-bit, and RGB volumes. Its 269 checks cover transfer
functions, threshold behaviour, black-pixel handling, colour-mode transitions,
and per-dataset state. A separate interface harness checks standalone Designer
forms, control sizes and text fit at 100%, 150%, and 200% scaling, preserved
dataset-colour styling, saved window layouts, and render-free VTK shutdown.
These are numerical and interface regression checks, not a benchmark or a
scientific rendered-image validation study. Windows memory checks can
reject an RGB-to-grayscale conversion whose estimated working set exceeds
available memory, but processing remains in-memory.

The current target is Windows x64 with Qt 5.15.2 and VTK 8.2. Portability,
out-of-core data handling, full scene serialization, and automated image
comparisons remain limitations. A repository
[architecture note](https://github.com/janlazer/UKE-OCTview3R/blob/main/docs/architecture.md)
documents filter chains, coordinate conventions, and test boundaries in detail.

# Research impact statement

The software originated in the Image-Guided Laser Surgery group at LZH. Möbes'
2014 project thesis, supervised by Hahn, established the C++/Qt/VTK volume,
slice, and colour-mapping workflow [@moebes2014]. Its integration into Hahn's
doctoral analysis environment provides documented research use: ex vivo
crystalline lenses imaged under simulated accommodation were displayed with
segmented anterior/posterior points, surfaces, and fitted ellipsoids
[@hahn2020, Sections 7.5 and 10.1.3]. These overlays supported visual inspection
of fitted geometry against the OCT signal. This historical application supports
the viewer's central concept, not validation of every feature in the current
version.

![Historical application to ex vivo crystalline-lens OCT. The original volume
is shown alone (a), with segmented anterior and posterior surface points (b),
as meshes and combined surfaces (c-d), and with a fitted ellipsoid represented
as points or a closed surface (e-f). Red and blue encode the anterior and
posterior lens surfaces. Reproduced from Figure 10.6 of [@hahn2020] by the
dissertation author.](figures/lens-oct-polydata-overlay.png){ width=100% }

Organoid imaging motivates a further application. Published OCT work includes
morphological assessment of patient-derived organoids [@zhang2023] and
longitudinal imaging of retinal organoids with dynamic full-field OCT
[@monfort2023]. These studies establish application context, not adoption of
`OCTview3R`. In the authors' ongoing study, OCT images of organoids in Matrigel
are combined with independently segmented geometry to inspect size, position,
and motion of surrounding organoids. Individual organoids are laser-ablated
and sampled for proteomic analysis. The proposed overlay documents spatial
context; segmentation, tracking, intervention, and proteomics remain external.

![Schematic workflow for the ongoing organoid study. Repeated OCT acquisition
and segmentation feed an OCTview3R overlay for visual inspection. The derived
geometry supports spatial characterization and supplies target context
for laser ablation, targeted sampling, and proteomics. The diagram shows the
workflow rather than quantitative results.](figures/organoid-study-workflow.png){ width=100% }

<!-- TODO before submission: add a publishable OCT/segmentation result image
from the Organoid Paper and cite that manuscript when a stable bibliographic
record is available. -->

Version 1.1.0 packages this research lineage as a documented, versioned,
GPL-3.0-licensed application prepared for public release. The organoid workflow
remains an ongoing application, without quantitative results claimed here.

# AI usage disclosure

OpenAI Codex (including GPT-5) assisted with software refactoring and review, repository
documentation, literature discovery and organization, and manuscript drafting
and formatting. Before submission, the
authors will review, modify where necessary, and validate all AI-assisted code,
documentation, claims, citations, and manuscript text, and will assume full
responsibility for the submitted work.

# Acknowledgements

The authors acknowledge the colleagues of the former Image-Guided Laser
Surgery group in the Department of Biomedical Optics at Laser Zentrum Hannover
e.V. for the scientific environment in which the original viewer was developed
and applied.

The authors thank Miroslav Zabic for his advice on preparing OCTview3R for
public release.

<!-- TODO before submission: add exact funders, grant identifiers, and the
sponsors' role; obtain confirmation from all authors for the conflict-of-interest
statement; replace future-tense wording in the AI disclosure after human review. -->

# References
