# Pre-publication history cleanup

Jan Hahn requested this cleanup on 15 September 2026, while the repository and
v1.1.0 release were still private/draft.

## Scope

- Remove previously deleted VTK/VTI/VTP/VTR examples from all locally published
  branch/tag histories, including earlier names and locations.
- Remove obsolete ImageData paths deleted during the author's example cleanup.
  Keep the current cherry TIFF/RAW files, their README and reference files still
  awaiting an explicit decision.
- Replace four obsolete personal computer paths in historical source text with
  relative `ImageData/` examples. Retain ordinary Windows/Qt/VTK build examples.
- Preserve the original commit sequence, authors, messages and timestamps;
  content changes necessarily produce new commit IDs. No earlier public
  development activity is implied.

The procedure uses a verified private Git bundle and an isolated mirror before
updating the working repository. The backup, removal list and commit-ID mapping
remain in ignored local storage; they must not be uploaded as release assets.
An unpublished v1.1.0 draft may be refreshed to the newly tested source revision;
the old tag and packages are retained in the private backup.

## Collaborator precautions

After the rewritten branch/tag are pushed, existing collaborators should make
a fresh clone after saving any uncommitted work. Do not merge an old clone into
the new branch, as that can reintroduce removed history. Existing private copies,
forks, Actions logs/artifacts and GitHub cached objects require separate review;
a force-with-lease push is not proof that every remote copy has been erased.

The live source snapshot is checked against the tested pre-rewrite tree, and
only explicit branch/tag refs are pushed with expected-old-value leases. There
is no blanket mirror push and no change to repository visibility.
