# Checks before changing repository visibility

Status recorded on 15 September 2026. The repository and v1.1.0 release remain
private/draft. This checklist is not a legal certification or a declaration
that every file is cleared. Publication is a separate author decision.

## Completed or confirmed

- [x] Jan Hahn confirmed that the retained TIFF/RAW examples show an ordinary
      market-bought cherry. The exact files are listed in
      [example-data provenance](../OCTview3R/ImageData/README.md).
- [x] Jan Hahn confirmed that he alone holds the dissertation rights and permits
      reuse of the selected lens figure.
- [x] Jan Hahn approved public inclusion of the organoid concept and supplied
      the 1 October 2026 GUI screenshot for the paper/README. Its caption states
      that cherry OCT and organoid meshes are independent datasets, not a
      registered pair. Each future scientific result image needs separate review.
- [x] Jan Hahn authorized removal of deleted VTK-family exports and obsolete
      personal paths from branch/tag history, with a private local backup.
      See [history-cleanup scope](history-cleanup.md).
- [x] Miroslav Zabic is acknowledged for publication advice in the About dialog,
      README and manuscript. His planned release test is not claimed as complete.
- [x] At Jan Hahn's request, remove `A03-R-039.pdf`, `ReadAllPolyDataDemo.pdf`,
      `file-formats.pdf` and `Example_CutMesh.txt` from the source tree,
      rewritten branch/tag history and refreshed release archives. Keep the
      original files only in a private local backup.

## Open before public release

- [ ] Obtain the co-authors' agreement to public software/documentation release
      under GPL-3.0-only and confirm authorship/affiliations. Jan will contact
      Tammo Ripken; a current contact for Giovanno Moebes has not been found.
      Resolve any relevant institutional/contractual questions with the authors.
- [ ] Complete a final review of identifying metadata, confidential material,
      Actions logs/artifacts and any external copies. Cleaning branch/tag history
      does not certify deletion from GitHub caches, forks or existing clones.
- [ ] After history cleanup, verify that the draft release's source archive,
      installer/portable manifests, tag and checksums identify the same revision.
      Rebuild packages rather than retaining an old source snapshot.
- [ ] Receive and record Miroslav's independent Windows installation/overlay
      test. See [release-test record](release-test.md). Local automated tests
      do not establish his result.
- [ ] Review new organoid result images, captions and acquisition/segmentation
      provenance before adding them. Concept approval is not approval of images
      that do not yet exist.

GitHub's [visibility guidance](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/managing-repository-settings/setting-repository-visibility)
describes the consequences of publication. See the separate
[JOSS checklist](../paper/SUBMISSION_CHECKLIST.md) for submission readiness.
