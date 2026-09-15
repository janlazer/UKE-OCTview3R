# Checks before changing repository visibility

This is a review checklist, not a declaration that every file is cleared.
The market-cherry TIFF provenance was confirmed by Jan Hahn on 15 September 2026.
No repository visibility change, file removal or history rewrite is performed by
adding this document. The paper and software release are separate decisions.

## Recorded observations

- The four TIFF examples listed in `OCTview3R/ImageData/README.md` are scans of a
  cherry purchased at a market, not human or clinical datasets.
- Other legacy files remain tracked: RAW, BYU, STL, PLY, XYZ, TXT, C++ example
  material and PDFs. The TIFF provenance does not by itself establish their origin
  or the redistribution terms of externally authored material.
- Previously removed VTK-family datasets remain in earlier commits. Current
  `.gitignore` rules are not a history-removal mechanism.
- The manuscript and its figures are tracked, including the historical lens
  figure and the ongoing organoid-workflow description.
- The prepared v1.1.0 installer/portable payload contains no research datasets.
  Its source archive was created from the tagged source revision and includes
  that revision's example/reference files. The draft release is not a rights audit.

## Human decisions and final verification

- [ ] Confirm the co-authors agree to public software/documentation release and
  verify that the stated GPL terms can be applied to their contributions. Resolve
  any institutional or contractual questions rather than inferring ownership from
  either an affiliation or private development alone.
- [ ] Classify each remaining example/reference file and record its provenance
  and permission, or decide that it should not be distributed.
- [ ] Resolve the dissertation figure's reuse terms and approve public disclosure
  of the ongoing organoid project description with the relevant collaborators.
- [ ] Check all reachable Git history, tags and release archives for credentials,
  identifying metadata, confidential research and material not cleared for sharing.
  No complete secret/rights audit has yet been certified.
- [ ] If something must be removed from history, agree an explicit scoped plan
  with collaborators, preserve a private backup, and update affected tags/archive
  references. Do not silently force-push or create a misleading development record.
- [ ] Review Actions logs/artifacts and release notes as well as current files.
  GitHub documents that Actions history/logs become public with the repository.
- [ ] Recreate affected source archives and checksums if the release contents
  change; do not present the old draft assets as a newly cleared source snapshot.
- [ ] Ask a colleague to install and inspect a representative TIFF/geometry
  overlay on a clean Windows computer. Record problems as real issues.

GitHub's [visibility guidance](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/managing-repository-settings/setting-repository-visibility)
describes the consequences. Public forks/copies cannot be recalled merely by
making the original repository private again.

For JOSS-specific requirements and author decisions, use
[SUBMISSION_CHECKLIST.md](../paper/SUBMISSION_CHECKLIST.md).
