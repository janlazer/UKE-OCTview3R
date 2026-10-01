# JOSS manuscript draft

This directory contains the draft manuscript for a possible submission to the
Journal of Open Source Software (JOSS). The structure follows the current JOSS
paper requirements while retaining the concise, figure-led style of the
OCTproZ JOSS paper.

## Files

- `paper.md`: manuscript source
- `paper.bib`: BibTeX references
- `../docs/developer-guide.md`: source-reading order, interface contracts and UI invalidation
- `../docs/architecture.md`: implementation-level companion covering ownership,
  filter chains, units, transparency, memory handling, and verification limits
- `figures/octview3r-interface.png`: current application screenshot, shared
  with the repository README so both use the same image
- `make_review_copy.py`: removes only the large DRAFT watermark from the
  JOSS-generated PDF, retaining all manuscript content and publication metadata
- `paper.pdf`: generated JOSS-format author-review copy without that watermark
  (ignored by Git); `paper-joss-draft.pdf` preserves the unmodified build
- `figures/overlay-pipeline.png`: OCT/PolyData rendering architecture
- `figures/lens-oct-polydata-overlay.png`: historical lens application from
  Figure 10.6 of Jan Hahn's dissertation
- `figures/organoid-study-workflow.png`: schematic of the ongoing organoid
  imaging, segmentation, ablation, and proteomics workflow (working project
  name: `Organoid-Paper`)
- `generate_diagrams.py`: reproducible generator for both block diagrams
- `render_review_pdf.py`: local review-PDF renderer for environments without
  the official JOSS/Inara toolchain
- `../output/pdf/octview3r-joss-draft.pdf`: generated review PDF (ignored by
  Git and reproducible from the source files above)

The review PDF is only a layout aid. The authoritative submission source is
`paper.md`; JOSS generates the publication proof with Inara.

## Source basis checked for this draft

- Jan Hahn's 2020 dissertation, DOI `10.15488/9863`, especially Sections 7.5
  and 10.1.3. These sections document OCT volume visualization and overlays of
  segmented anterior and posterior crystalline-lens surfaces.
- Giovanno Möbes' project thesis dated 23 April 2014, *Realisierung einer
  Volume Rendering Software in C++ mit Qt und dem Visualization Toolkit
  (VTK)*. It documents the original Qt/VTK architecture, OCT motivation, volume
  rendering, variable planes, thresholds, and colour maps.
- The current OCTview3R 1.1.0 implementation and repository documentation.
  This includes the grayscale/RGB volume paths, luminance-derived RGB alpha
  masking, independent palette autoscaling and window/level controls, smooth
  scalar opacity, coordinate conventions, and the existing regression harness.
- The 1 October 2026 interface update: compact dark-only styling, independently
  previewable Designer forms, GUI checks at three display scales, and VTK
  shutdown checks. These interface tests are not scientific image validation.
- Jan Hahn supplied the updated interface screenshot on 1 October 2026 for
  the README and paper. It shows the cherry example volume and independently
  imported organoid meshes in one scene, not a registered pair from the same
  specimen. It is an interface illustration, not an organoid result figure;
  the underlying organoid meshes are not distributed.
- Eight additional references checked on 14 September 2026: direct volume
  rendering (Levoy, 1988), mixed polygon/volume rendering (Levoy, 1990), transfer
  functions (Kindlmann and Durkin, 1998), OCT speckle (Schmitt et al., 1999),
  Fiji (Schindelin et al., 2012), organoid OCT morphology (Zhang et al., 2023),
  longitudinal organoid imaging (Monfort et al., 2023), and scientific colour
  mapping (Crameri et al., 2020). The bibliography now contains 19 cited works,
  including two official extension-documentation references added on 15 September;
  DOI metadata and primary author/publisher records were used to check them.
  The organoid references provide background, not evidence that those groups
  used OCTview3R.
- The current JOSS paper, review, AI-disclosure, and pre-submission criteria.

The ParaView/3D Slicer comparison acknowledges their extension capabilities and
explains the choice of an independent, focused Qt/VTK application. Informal
reports of crashes or difficult plane interaction are not treated as comparative
evidence: a stability claim would need versions, datasets, hardware and
reproducible steps. No superiority in stability or usability is asserted.

The two source PDFs are not versioned or redistributed with the repository.

## Items to resolve before submission

See [SUBMISSION_CHECKLIST.md](SUBMISSION_CHECKLIST.md) for the dated readiness
assessment, author decisions, public-development requirement, and submission
route. A prepared GitHub software release is not a JOSS submission.
Also complete the [public-release checks](../docs/public-release-checklist.md)
before changing repository visibility. The ImageData examples are limited to
documented cherry TIFF/RAW data; legacy PDF/text references have been removed.

- Confirm the author order and each author's current submission affiliation.
- Add ORCIDs for Giovanno Möbes and Tammo Ripken if available.
- Add an email address for the corresponding author if requested by JOSS.
- Add exact funding bodies, grant identifiers, and the sponsors' role.
- Obtain a conflict-of-interest confirmation from every author.
- Complete human review of all AI-assisted code and manuscript content, then
  update the AI disclosure from future to past tense.
- Add a current, publishable OCT/segmentation result image from the organoid
  study and cite the associated Organoid Paper once its bibliographic record is
  available. The included diagram currently documents the workflow only.
- Jan Hahn confirmed on 15 September 2026 that he alone holds the dissertation
  rights and permits reuse of the selected figure. He also approved public
  inclusion of the organoid concept. The supplied interface illustration does
  not replace a matched organoid OCT/segmentation result image, which still
  needs its own provenance, caption and approval before it is added.
- Expand the existing automated regression tests and add CI. The current
  numerical/UI checks do not replace rendered-image comparisons or benchmarks.
- Plan for the public development history required by
  [JOSS's submission criteria](https://joss.readthedocs.io/en/latest/submitting.html):
  software developed privately needs at least six months of public development
  history before submission. Changing repository visibility is a separate
  author decision; this manuscript update does not publish the repository.
- Demonstrate continued research use before JOSS submission.
- At the end of review, tag the accepted release and archive it with Zenodo or
  another long-term repository to obtain a software DOI.

## Official build

The `JOSS paper draft` GitHub Actions workflow runs manuscript consistency
checks and the official Open Journals PDF generator. It is also available via
**Actions > JOSS paper draft > Run workflow**. Download the resulting
`octview3r-joss-paper` artifact and inspect `paper.pdf`. The artifact also keeps
the original `paper-joss-draft.pdf`. The review-copy step removes only the large
DRAFT watermark; it does not change the unpublished status, invent a DOI, or
turn the output into an accepted journal proof. This workflow is paper CI,
not a Windows C++/Qt/VTK test runner, and never submits the paper to JOSS.

To reproduce the watermark-free copy locally, install the workflow's pinned
`pypdf` dependency and run:

```powershell
python .\paper\make_review_copy.py .\paper\paper-joss-draft.pdf .\paper\paper.pdf
```

Use the JOSS `inara` toolchain described in the JOSS documentation when Docker
or the required container runtime is available. The local review renderer can
be run with the bundled Python environment:

```powershell
python .\paper\render_review_pdf.py
```

The local renderer supports the manuscript's technical subsections, linked
architecture note, and author-year citations, checks for missing citation keys,
and sorts the reference list alphabetically. It is not a general BibTeX parser:
reference fields should remain single-line, brace-delimited values. The main
text remains within the [JOSS paper length guidance](https://joss.readthedocs.io/en/latest/paper.html);
longer implementation details belong in the architecture companion.
