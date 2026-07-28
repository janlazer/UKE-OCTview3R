# OCTview3R

OCTview3R is a Windows desktop application for interactive visualization of
volumetric image data and polygonal data sets. It is written in C++ and uses
Qt for the user interface and VTK for data processing and 3D rendering.

The repository contains the original application sources, Qt Designer forms,
icons, and a collection of example data sets.

## Features

- Volume rendering for RAW, TIFF, JPEG stack, and legacy VTK image data
- Polygonal and point-data import for VTK, STL, PLY, VTP, OBJ, BYU (`.g`),
  VTR, and XYZ files
- Maximum-intensity, composite, additive, and minimum-intensity blend modes
- Threshold, opacity, color-transfer, and polygonal gloss controls
- Per-axis range cropping for volume and polygonal data
- Interactive image plane with optional median filtering and clipping
- Multiple data sets in a tabbed user interface
- Object translation, rotation, and axis scaling
- Camera presets, orientation marker, scalar bar, and cube axes
- TIFF export of the current render window

## Requirements

The current project configuration targets the following toolchain:

- Windows x64
- Visual Studio 2022 with the MSVC v143 toolset
- Windows 10 SDK
- Qt 5.15.2 for `msvc2019_64`
- Qt Visual Studio Tools / Qt MSBuild integration
- VTK 8.2.0 built for x64 with Qt and OpenGL support

Qt and VTK must use ABI-compatible compiler and runtime settings.
This matches the VTK, Qt, and MSVC configuration used by the
`OpticalSampleScannerJob` in `UKE-smartLab`.

## Building

1. Clone the repository:

   ```powershell
   git clone https://github.com/janlazer/UKE-OCTview3R.git
   cd UKE-OCTview3R
   ```

2. Configure the dependency paths. The project expects these environment
   variables:

   ```powershell
   $env:VTKDIR = "C:\Programming\VTK\include"
   $env:VTKLIB = "C:\Programming\VTK\lib"
   $env:VTKBIN = "C:\Programming\VTK\bin"
   ```

   The project automatically selects the `Debug` or `Release` subdirectory
   below `VTKLIB` and `VTKBIN`. A configuration-specific directory can also
   be supplied directly. Do not add both VTK runtime configurations to the
   global `PATH`; the project prepends only the active configuration.

   This follows the `UKE-smartLab` dependency layout:

   - `VTKDIR` contains the VTK 8.2.0 headers.
   - `VTKLIB\Debug` and `VTKLIB\Release` contain the matching import libraries.
   - `VTKBIN\Debug` and `VTKBIN\Release` contain the matching runtime DLLs.

3. Open `OCTview3R.sln` in Visual Studio.

4. Select `Release | x64` or `Debug | x64` and build the solution. The selected
   configuration determines which VTK library subdirectory is used.

The Release configuration can also be built from a Visual Studio developer
shell:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  .\OCTview3R.sln `
  /t:Build `
  /p:Configuration=Release `
  /p:Platform=x64 `
  /p:VTKLIB="C:\Programming\VTK\lib"
```

## Running

Use the configuration-aware launcher to avoid mixing Debug and Release DLLs:

```powershell
.\run-viewer.ps1 -Configuration Release
```

The launcher prepends `VTKBIN\<Configuration>` and the Qt DLL directory only
for the child process.

## Deployment

Create a deployable runtime directory with Qt plugins and the matching VTK
DLLs:

```powershell
.\deploy-runtime.ps1 -Configuration Release
```

The result is written to `dist\Release`. The same deployment can be invoked as
an MSBuild target:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  .\OCTview3R.sln `
  /t:DeployRuntime `
  /p:Configuration=Release `
  /p:Platform=x64
```

`windeployqt` also places `vc_redist.x64.exe` in the deployment directory.
Install it once on target systems that do not already provide the matching
Microsoft Visual C++ runtime.

## Project Structure

```text
OCTview3R.sln
run-viewer.ps1             Configuration-safe development launcher
deploy-runtime.ps1         Qt/VTK runtime deployment
OCTview3R/
  OCTview3R.cpp/.h       Main window and UI event handling
  documentModel.cpp/.h   Data-set ownership and active-document selection
  viewerController.cpp/.h Renderer and global viewer decorations
  volumePipeline.cpp/.h  Volume filtering, transfer functions, and planes
  polyPipeline.cpp/.h    Polygonal-data rendering
  transformableImagePlaneWidget.cpp/.h
                         Plane rendering and interaction in object space
  viewerData.h           Per-data-set viewer state and owned VTK objects
  loading.cpp/.h         Background data loading
  opendata.cpp/.h/.ui    Volume-data import dialog
  openpoly.cpp/.h/.ui    Polygonal-data import dialog
  OCTview3R.ui           Main Qt Designer form
  Resources/             Icons and color-map resources
  ImageData/             Example volume and geometry data
```

## Development Status

This is a legacy research codebase and currently has no automated test suite.
The application builds successfully with the reference dependency versions
above, but several areas need further work before critical use:

- VTR and other scientific-data imports need broader format coverage tests.
- Tab removal and repeated transform/render operations need regression tests.
- Pipeline components do not yet have automated unit or image-regression tests.
- A future Qt/VTK upgrade should be handled as a dedicated migration because
  both frameworks have breaking API changes beyond the reference versions.

When adding new functionality, prefer small, testable pipeline components and
VTK smart pointers over additional raw-pointer ownership.

## License

No license file is currently included. Add an appropriate license before
redistributing or reusing the project outside its intended environment.
