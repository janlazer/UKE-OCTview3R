#pragma once

#include <QString>
#include <vtkSmartPointer.h>
#include <functional>

class vtkImageData;

namespace ThorlabsOct
{
struct Metadata
{
    int dimensions[3] = { 1, 1, 1 }; // Viewer order X, Y, Z (axial).
    double spacing[3] = { 1.0, 1.0, 1.0 }; // mm; Range/Size, not Range/(Size-1).
    unsigned int calibratedAxes = 0;
};

// Read only the ZIP directory and bounded XML header; suitable for the dialog.
bool inspect(const QString& path, Metadata& metadata, QString& error);

// Stream little-endian float32 dB intensity into one VTK allocation, transposing
// Thorlabs' Z-fast layout to VTK's X-fast layout. Never extracts files to disk.
// Failure leaves output null. A supplied memory budget makes rejection testable;
// zero uses the available physical memory on Windows, including a render reserve.
bool read(const QString& path, vtkSmartPointer<vtkImageData>& output,
    Metadata& metadata, QString& error,
    const std::function<void(int)>& progress = {}, quint64 memoryBudget = 0);
}
