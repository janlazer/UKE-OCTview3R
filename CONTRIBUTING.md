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

## Bug reports

Include the OCTview3R version, build configuration, Windows version, Qt and
VTK versions, exact reproduction steps, and the input format. Attach only
data that you are legally permitted to publish; a minimal synthetic example
is preferred.

## License of contributions

By submitting a contribution, you agree that it may be distributed under the
project's GNU General Public License version 3 only (`GPL-3.0-only`).
