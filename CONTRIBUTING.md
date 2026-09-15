# Contributing

Contributions to OCTview3R are welcome through GitHub issues and pull
requests.

## Before submitting a change

- Build the affected `Debug | x64` or `Release | x64` configuration.
- Keep user-interface layout changes in the corresponding `.ui` file whenever
  Qt Designer can represent them.
- Keep data-processing and rendering changes in the focused pipeline classes.
- Do not submit patient data, identifying metadata, credentials, proprietary
  sample files, or dependency binaries.
- Document externally derived code or assets and retain their license notices.
- Run `tests/run-threshold-regression.ps1` for pipeline or interface changes.
  For manuscript changes, run `python -B tests/check-paper.py` and inspect the
  `JOSS paper draft` Actions artifact.

## Support and review

Use GitHub issues for reproducible problems and feature proposals; pull requests
are reviewed by the project maintainers. This research project has no guaranteed
response time or commercial support commitment. Maintainers decide whether a
change fits the viewer's scope. Never attach private research data to a public
discussion; the security policy covers sensitive vulnerability reports.

## Bug reports

Include the OCTview3R version, build configuration, Windows version, Qt and
VTK versions, exact reproduction steps, and the input format. Attach only
data that you are legally permitted to publish; a minimal synthetic example
is preferred.

## License of contributions

By submitting a contribution, you agree that it may be distributed under the
project's GNU General Public License version 3 only (`GPL-3.0-only`).
