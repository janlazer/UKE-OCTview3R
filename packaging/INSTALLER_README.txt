OCTview3R for Windows
===================

This installer includes OCTview3R, Qt, VTK, Qt plugins, and the application-local
Microsoft Visual C++/OpenMP runtime. No separate Qt, VTK, Visual Studio, Python,
or Internet connection is needed to install and run the application.

Requirements: Windows 10/11 x64, a suitable OpenGL graphics driver, and enough
memory for your datasets. Windows on ARM is not a supported target.

Installation is for the current Windows user and normally requires no
administrator rights. Start OCTview3R from the Start menu. A desktop shortcut
is optional. The installer does not change PATH or file associations.

To update, close OCTview3R and run the newer installer. To uninstall, use
Windows Settings > Apps > Installed apps. Personal datasets and existing
application preferences are not deliberately removed by the uninstaller.
Keep research data outside the application installation directory.

The installer is not code-signed. Windows may display an unknown-publisher or
SmartScreen warning. Obtain the package only from the project's official
GitHub release and verify its SHA-256 checksum. Do not disable Windows security.
Institutional policies may require installation approval from your IT team.

No research datasets are bundled. Use File > Open Volume or Open PolyData to
load your own files. See About for authors, version, and application licensing.

License: GPL-3.0-only. Dependencies retain their own licenses; see LICENSE.txt,
THIRD_PARTY_NOTICES.md, and licenses/. Source code and release downloads:
https://github.com/janlazer/UKE-OCTview3R/releases
While the repository is private, collaborators need a GitHub invitation.
