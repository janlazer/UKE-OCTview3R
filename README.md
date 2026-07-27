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
- Threshold, opacity, and color-transfer controls
- Interactive image plane with optional median filtering and clipping
- Multiple data sets in a tabbed user interface
- Object translation, rotation, and axis scaling
- Camera presets, orientation marker, scalar bar, and cube axes
- TIFF export of the current render window

## Requirements

The current project configuration targets the following toolchain:

- Windows x64
- Visual Studio with the MSVC v142 toolset
- Windows 10 SDK
- Qt 5.15.2 for `msvc2019_64`
- Qt Visual Studio Tools / Qt MSBuild integration
- VTK 8.2 built for x64 with Qt and OpenGL support

Visual Studio 2022 can build the project when the v142 toolset is installed.
Qt and VTK must use ABI-compatible compiler and runtime settings.

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
   $env:VTKLIB = "C:\Programming\VTK\lib\Release"
   $env:VTKBIN = "C:\Programming\VTK\bin"
   ```

   `VTKLIB` must point to the configuration-specific directory that contains
   the VTK `.lib` files. Pointing it only to `C:\Programming\VTK\lib` causes
   linker error `LNK1181`.

3. Open `OCTview3R.sln` in Visual Studio.

4. Select `Release | x64` or `Debug | x64` and build the solution. For a Debug
   build, set `VTKLIB` to the corresponding VTK Debug library directory.

The Release configuration can also be built from a Visual Studio developer
shell:

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" `
  .\OCTview3R.sln `
  /t:Build `
  /p:Configuration=Release `
  /p:Platform=x64 `
  /p:VTKLIB="C:\Programming\VTK\lib\Release"
```

## Running

Ensure that the Qt and configuration-specific VTK DLL directories are on
`PATH`, then start the generated executable:

```powershell
$env:PATH = "C:\Programming\VTK\bin\Release;C:\Programming\Qt\5.15.2\msvc2019_64\bin;$env:PATH"
.\x64\Release\OCTview3R.exe
```

## Project Structure

```text
OCTview3R.sln
OCTview3R/
  OCTview3R.cpp/.h       Main window and visualization pipeline
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

- VTR import requires correction and additional format tests.
- Thread and VTK object lifetimes should be converted to explicit RAII
  ownership.
- Tab removal and repeated transform/render operations need regression tests.
- Experimental routines with machine-specific paths should be removed or
  isolated from the production interface.

When adding new functionality, prefer small, testable pipeline components and
VTK smart pointers over additional raw-pointer ownership.

## License

No license file is currently included. Add an appropriate license before
redistributing or reusing the project outside its intended environment.
