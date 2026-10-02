# JOSS submission readiness

Checked against the repository and current guidance on 15 September 2026.
The prepared software release is not a JOSS submission or an acceptance claim.

## Current assessment

The repository is still private. JOSS requires an openly accessible repository
and more than six months of active public development. Private commit dates do
not establish that public record. If public development started on 15 September
2026, submission would be considered only after 15 March 2027, with the other
criteria also satisfied. This is an eligibility boundary, not a guaranteed
acceptance date. [Submission criteria](https://joss.readthedocs.io/en/latest/submitting.html)

Already present in OCTview3R:

- GPL-3.0-only license, author/citation metadata, CONTRIBUTING and SECURITY files.
- A 1.1.0 Windows installer/portable packaging workflow, dependency notices,
  checksums, installation tests, and documented build instructions.
- 269 synthetic production-pipeline and Qt-control regression checks.
- A roughly 1,700-word manuscript with the required sections, 21 references,
  four figures, and an implementation-level architecture companion.
- Documented historical lens-imaging use in Hahn's dissertation. The organoid
  workflow is identified as ongoing, not presented as a completed validation.
- A GitHub Actions workflow for manuscript checks and an official JOSS-format
  PDF. Its run must succeed; configuring it alone is not a successful build.

## Decisions and checks for the human authors

- [ ] Approve author order and describe each person's actual contribution;
      confirm current versus historical affiliations. Jan's ORCID is present;
      add the others if available. Confirm the submitting author's contact data.
- [ ] Supply accurate funding and conflict-of-interest statements. Private
      development alone does not establish that all research or infrastructure
      was unfunded. Do not insert an assumed "no conflicts" declaration.
- [ ] Review all AI-assisted code, figures, references, and manuscript claims.
      Replace the future-tense AI disclosure only after this review has actually
      happened, and identify the tools/models as accurately as the records allow.
- [x] Jan Hahn confirmed on 15 September 2026 that he alone holds the dissertation
      rights and permits reuse of the selected figure.
- [x] Jan Hahn approved public inclusion of the organoid concept and supplied
      an interface illustration on 1 October 2026, subsequently revised to show
      a grayscale volume with red surfaces and blue meshes. This does not verify
      registration accuracy; clear each future scientific result image separately.
- [ ] Decide whether to add a publishable organoid OCT/segmentation overlay with
      calibration, acquisition context, segmentation provenance, and explicit
      consent to disclose the image. A representative result would strengthen
      the use case; a second completed science paper is not itself mandatory.
- [ ] Make the build-versus-contribute argument more concrete if possible: which
      lens/organoid inspection steps motivated this particular GUI and pipeline?
      The comparison now acknowledges ParaView/Slicer extensions and explains
      source-level customization of the independent application. Confirm that
      this rationale represents the authors' actual design decisions. Do not
      claim superior stability, speed or accuracy without measurements.

The manuscript already contains comments marking unresolved author information.
The JOSS format requires disclosure of AI use and verification, accurate metadata,
and acknowledgement of funding. [Paper format](https://joss.readthedocs.io/en/latest/paper.html)

## Software/community work still useful before review

- [ ] Record a colleague's installation and representative TIFF/PolyData overlay
      test on a clean Windows computer. Miroslav Zabic is the planned tester;
      his test is pending. The local packaging test is not that test.
- [ ] Expand automated coverage of calibration, mesh imports, transformations,
      and mixed volume/geometry occlusion. Rendered-image baselines are currently
      missing; the 269 assertions do not provide image-quality validation.
- [ ] Add a reproducible C++/Qt/VTK CI environment. The paper workflow does not run
      the Windows application tests and must not be represented as software CI.
- [ ] Provide a reproducible, shareable overlay example. Current bundled data
      include non-medical market-cherry TIFF/RAW files. Deleted VTK exports and
      former PDF/text references are removed from the rewritten history. See the
      remaining [public-release checks](../docs/public-release-checklist.md).
- [ ] Document real public bug reports, discussions, contributions, and releases
      as they occur. Do not manufacture activity to satisfy an eligibility check.

Installation, usage examples, contribution/support routes, and objective checks
are reviewed alongside the paper. [Review criteria](https://joss.readthedocs.io/en/latest/review_criteria.html)

## Where and how to submit

When eligible, sign in with GitHub at [JOSS](https://joss.theoj.org/) and choose
[Submit](https://joss.theoj.org/papers/new). Supply the repository URL, version,
and manuscript location `paper/paper.md`; follow the current form's prompts.
The review is handled publicly in GitHub issues. Submission and publication
are free. Archive the accepted software version for its software DOI after
review, as directed by the editor. [Submission process](https://joss.readthedocs.io/en/latest/submitting.html#submission-process)

The local review PDF is not the submission source. Run **Actions > JOSS paper
draft > Run workflow** and inspect the `octview3r-joss-paper` artifact before
submission. The workflow uses the official
[Open Journals PDF generator](https://github.com/marketplace/actions/open-journals-pdf-generator).

Current JOSS policy does not permit AI-generated author/editor/reviewer
conversation, except translation. The authors must conduct that correspondence
themselves. [AI policy](https://joss.readthedocs.io/en/latest/submitting.html#ai-usage-policy)
