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
date: 19 August 2026
bibliography: paper.bib
---

# Summary

Optical coherence tomography (OCT) provides micrometre-scale cross-sectional
images from optically scattering samples [@huang1991]. Three-dimensional OCT
studies commonly produce two complementary kinds of data: volumetric image
stacks that retain the measured intensities and geometric representations such
as point clouds or surface meshes obtained by segmentation. Inspecting either
representation alone can conceal segmentation errors, registration offsets, or
the spatial relationship between an identified structure and the surrounding
signal.

`OCTview3R` is a graphical Windows application for interactive, offline
visualization of volumetric image data together with polygonal or point data. It
is written in C++, uses Qt for its user interface, and builds its rendering
pipelines with the Visualization Toolkit (VTK) [@schroeder1996; @vtkbook2006].
Multiple datasets are placed in a shared three-dimensional scene so that, for
example, a TIFF stack containing OCT intensities can be overlaid with anterior
and posterior surfaces exported by a segmentation workflow. Users can adjust
visibility, colour, opacity, thresholds, crop ranges, and object transforms, and
can inspect the volume with an interactive slice plane. Single-component and
RGB stacks are supported, allowing scalar OCT data and colour-coded volumetric
results to be examined in the same application. The software does not perform
segmentation; it is a focused viewer for evaluating and communicating the
outputs of image-analysis workflows.

![OCTview3R 1.1.0 displaying a non-medical OCT volume of a cherry. The dark
theme is optional; the left-hand controls expose dataset metadata, rendering
parameters, transforms, and crop ranges.](figures/octview3r-interface.png){ width=100% }

# Statement of need

Researchers developing OCT analysis pipelines need a rapid visual check between
the original volume and derived geometry. A list of segmented coordinates may
be numerically valid but displaced, mirrored, truncated, or fitted poorly near a
boundary. A conventional two-dimensional slice viewer makes these defects hard
to assess across the full acquisition, while a surface-only view removes the
intensity context needed to judge the segmentation. `OCTview3R` addresses this
gap by displaying both representations in one manipulable scene.

The intended users are researchers and engineers who work with OCT or similar
three-dimensional image stacks and exchange intermediate results through files.
The viewer accepts RAW data with explicit dimensions, TIFF and JPEG stacks, and
legacy VTK image data. Geometry can be loaded from VTK PolyData, STL, PLY, VTP,
OBJ, BYU, VTR, and XYZ files. Volumes support composite, additive, minimum-
intensity, and maximum-intensity rendering. Geometry can be shown as points,
wireframes, or surfaces. Per-dataset translation, rotation, axis scaling,
opacity, colour, and spatial cropping allow pre-registered data to be checked or
small alignment differences to be explored interactively. These capabilities
make the application useful for quality control, figure preparation, and
discussion of segmentation results without requiring users to construct a
general-purpose visualization pipeline.

Multi-component image data is detected automatically and can be displayed as
RGB or as luminance-derived grayscale. In RGB mode, one window/level adjustment
is applied consistently to all three colour channels. A luminance-derived mask
drives thresholding and volume opacity, making black and rejected voxels
transparent without replacing the colours of accepted voxels. The interactive
slice plane retains opaque black pixels so that it remains a faithful image
view. Composite blending is recommended when preservation of the original RGB
appearance is more important than intensity projection.

# State of the field

Several established open-source tools cover adjacent needs. `OCTproZ` performs
live OCT acquisition, GPU signal processing, and visualization, and exposes a
plug-in system for hardware and processing extensions [@zabic2020]. In contrast,
`OCTview3R` starts after acquisition and processing; its contribution is the
lightweight joint inspection of stored volumes and independently generated
geometric data. ParaView provides a broad, scalable environment for scientific
visualization [@ahrens2005], while 3D Slicer and ITK-SNAP provide comprehensive
medical-image computing and segmentation environments [@fedorov2012;
@yushkevich2006]. These packages are appropriate when users need extensible
analysis pipelines, DICOM-centred workflows, distributed rendering, or
segmentation algorithms.

`OCTview3R` deliberately occupies a narrower role. It exposes the parameters
needed for an OCT volume/segmentation comparison directly in a compact desktop
interface and preserves interoperability through common image and geometry
formats. Extending a large platform or an acquisition application would have
coupled this small post-processing task to substantially broader workflows.
The focused design trades scripting, automated registration, and distributed
processing for a short path from files to an interactive overlay. It therefore
complements rather than replaces the general and modality-specific tools above.

# Software design

The current architecture separates dataset ownership, VTK processing, and user
interaction. A `DocumentModel` owns one state object per loaded volume or
geometric dataset. A `ViewerController` maintains a single VTK renderer and
dispatches updates to dedicated `VolumePipeline` and `PolyPipeline` components.
Because all dataset actors and volumes enter the same renderer, any number of
loaded segmentations can be viewed against one or more intensity volumes.

Each state object owns its VTK resources through smart pointers and records
appearance, transform, crop, and plane settings. Dirty flags distinguish data-
pipeline changes from inexpensive appearance or transform updates, avoiding a
complete pipeline reconstruction after every user-interface event. Volume
cropping uses a volume of interest, whereas polygonal cropping uses a clipping
box. A common object transform is applied to the rendered volume or polygonal
actor, and the interactive image plane is transformed in the same object space;
this keeps volume, plane, and derived geometry spatially coherent during
rotation, translation, and anisotropic scaling.

The scalar-volume path applies thresholding before transfer-function mapping.
The RGB path extracts the first three components, derives luminance for its
binary alpha mask, applies a shared window/level mapping to the colour channels,
and assembles an RGBA volume for VTK's dependent-component rendering. Both
paths use the same crop, transform, plane, and renderer infrastructure, and the
selected colour mode is stored per dataset.

![Volumetric OCT and independently generated segmentation geometry pass
through dedicated pipelines and enter one VTK renderer. Shared object-space
transforms keep the volume, interactive plane, and PolyData spatially coherent
for overlay inspection.](figures/overlay-pipeline.png){ width=100% }

Data loading is separated from the main interface and reports progress for
readers that expose it. Qt Designer forms define the principal dialogs and make
the visual layout inspectable without running the application. System, light,
and dark themes, camera presets, orthographic projection, orientation aids, and
numeric controls support both exploratory use and consistent figure framing.
The current version targets Qt 5.15.2 and VTK 8.2 on Windows x64. This choice
matches the surrounding laboratory software environment, but it is also a
current limitation: portability, session serialization, and automated image-
regression testing remain future work.

# Research impact statement

The software lineage began in the Image-Guided Laser Surgery group at LZH. A
2014 project thesis by Möbes, supervised by Hahn, implemented a C++/Qt/VTK
volume-rendering application for OCT data and established the interactive
volume, slice, threshold, and colour-mapping workflow [@moebes2014]. This work
was subsequently integrated into the computational analysis environment used
for Hahn's doctoral research.

In that research, the viewer was applied to ex vivo crystalline-lens OCT data
acquired under simulated accommodation [@hahn2020, Sections 7.5 and 10.1.3].
Anterior and posterior lens surfaces were extracted as segmented point lists
and fitted ellipsoids. Overlaying these data in different colours with the OCT
stack made it possible to inspect the fit near the lens boundary, switch between
points, meshes, and closed surfaces, and compare accommodated and
de-accommodated states. The published dissertation therefore documents a
realized scientific use of the viewer's central concept: interpreting segmented
three-dimensional regions in the context of the underlying OCT signal.

![Historical application to ex vivo crystalline-lens OCT. The original volume
is shown alone (a), with segmented anterior and posterior surface points (b),
as meshes and combined surfaces (c-d), and with a fitted ellipsoid represented
as points or a closed surface (e-f). Red and blue encode the anterior and
posterior lens surfaces. Reproduced from Figure 10.6 of [@hahn2020] by the
dissertation author.](figures/lens-oct-polydata-overlay.png){ width=100% }

An ongoing organoid study provides a second application domain. Three-
dimensional OCT volumes are acquired from organoids embedded in Matrigel, and
individual organoids are segmented to determine their size and position and to
analyse the motion of neighbouring organoids across repeated acquisitions.
Overlaying these segmentations as PolyData with the OCT intensities provides a
direct visual check of object identity and spatial context. The same context is
used when selecting individual organoids for laser ablation, targeted sampling,
and subsequent proteomic analysis. The block diagram below separates these two
linked outcomes without implying that OCTview3R itself performs segmentation,
motion estimation, ablation, or proteomics.

![Schematic workflow for the ongoing organoid study. Repeated OCT acquisition
and segmentation feed an OCTview3R overlay for visual validation. The derived
geometry supports spatial characterization and supplies verified target context
for laser ablation, targeted sampling, and proteomics. The diagram shows the
workflow rather than quantitative results.](figures/organoid-study-workflow.png){ width=100% }

<!-- TODO before submission: add a publishable OCT/segmentation result image
from the Organoid Paper and cite that manuscript when a stable bibliographic
record is available. -->

Version 1.1.0 turns this research-specific lineage into a documented,
versioned, GPL-3.0-licensed application prepared for public release. Its main
near-term role is visual quality control for studies in which segmented
structures must remain traceable to their source volume, including planned
applications that combine OCT data with independently segmented tissue regions.

# AI usage disclosure

OpenAI Codex using GPT-5 assisted with refactoring and review of parts of the
software, preparation of repository documentation, literature organization,
and initial drafting and formatting of this manuscript. Architectural and
scientific decisions remained with the human authors. Before submission, the
authors will review, modify where necessary, and validate all AI-assisted code,
documentation, claims, citations, and manuscript text, and will assume full
responsibility for the submitted work.

# Acknowledgements

The authors acknowledge the colleagues of the former Image-Guided Laser
Surgery group in the Department of Biomedical Optics at Laser Zentrum Hannover
e.V. for the scientific environment in which the original viewer was developed
and applied.

<!-- TODO before submission: add exact funders, grant identifiers, and the
sponsors' role; obtain confirmation from all authors for the conflict-of-interest
statement; replace future-tense wording in the AI disclosure after human review. -->

# References
