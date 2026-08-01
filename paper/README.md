# JOSS manuscript draft

This directory contains the draft manuscript for a possible submission to the
Journal of Open Source Software (JOSS). The structure follows the current JOSS
paper requirements while retaining the concise, figure-led style of the
OCTproZ JOSS paper.

## Files

- `paper.md`: manuscript source
- `paper.bib`: BibTeX references
- `figures/octview3r-interface.png`: current application screenshot
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
- The current JOSS paper, review, AI-disclosure, and pre-submission criteria.

The two source PDFs are not versioned or redistributed with the repository.

## Items to resolve before submission

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
- Confirm that the selected dissertation figure may be reproduced under the
  publication agreement with TEWISS Verlag; Jan Hahn has approved its use as
  the dissertation author.
- Add automated tests and CI, document an active public development history,
  and demonstrate continued research use before JOSS submission.
- At the end of review, tag the accepted release and archive it with Zenodo or
  another long-term repository to obtain a software DOI.

## Official build

Use the JOSS `inara` toolchain described in the JOSS documentation when Docker
or the required container runtime is available. The local review renderer can
be run with the bundled Python environment:

```powershell
python .\paper\render_review_pdf.py
```
