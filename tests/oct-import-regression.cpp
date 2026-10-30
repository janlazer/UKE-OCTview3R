#include "../OCTview3R/thorlabsOctReader.h"
#include "../OCTview3R/loading.h"
#include "../OCTview3R/opendata.h"
#include "../OCTview3R/documentModel.h"
#include "../OCTview3R/viewerData.h"
#include "../OCTview3R/volumePipeline.h"
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QSettings>
#include <QSpinBox>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkPiecewiseFunction.h>
#include <vtkColorTransferFunction.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>

namespace {
int checks = 0;
void require(bool ok, const char* message)
{
    ++checks;
    if (!ok) throw std::runtime_error(message);
}
void near(double a, double b, const char* message)
{
    require(std::abs(a - b) < 1e-6, message);
}
QString fixture(const char* name)
{
    return QDir::currentPath() + "/tmp/oct-regression/fixtures/" + name;
}
void fixtures()
{
    for (const char* name : {"stored.oct", "deflate.oct", "local-zip64.oct", "zip64.oct", "descriptor.oct", "backslash.oct"}) {
        ThorlabsOct::Metadata metadata;
        QString error;
        require(ThorlabsOct::inspect(fixture(name), metadata, error), qPrintable(error));
        require(metadata.dimensions[0] == 3 && metadata.dimensions[1] == 2 && metadata.dimensions[2] == 4,
            "correct X/Y/Z dimensions");
        require(metadata.calibratedAxes == 7, "all axes calibrated");
        for (double spacing : metadata.spacing) near(spacing, 0.1, "DataFile ranges override optical depth without double correction");
        vtkSmartPointer<vtkImageData> image;
        std::vector<int> progress;
        require(ThorlabsOct::read(fixture(name), image, metadata, error,
            [&](int p) { progress.push_back(p); }), qPrintable(error));
        require(image->GetScalarType() == VTK_FLOAT && image->GetNumberOfScalarComponents() == 1,
            "preserve scalar float32, not quantized grayscale");
        for (double origin : {image->GetOrigin()[0], image->GetOrigin()[1], image->GetOrigin()[2]})
            near(origin, 0, "origin remains zero");
        for (int z = 0; z < 4; ++z)
            for (int y = 0; y < 2; ++y)
                for (int x = 0; x < 3; ++x)
                    near(image->GetScalarComponentAsDouble(x, y, z, 0), -20 + 20*y + 4*x + z*0.25,
                        "lossless ZXY to XYZ transpose");
        require(progress.front() == 0 && progress.back() == 100 && progress.size() > 2, "real progress starts/advances/finishes");
        for (size_t i = 1; i < progress.size(); ++i)
            require(progress[i] >= progress[i-1] && progress[i] <= 100, "bounded monotonic progress");
    }
    for (const char* name : {"wrong-size.oct", "wrong-axis.oct", "time-series.oct", "freeform.oct",
        "bad-range.oct", "bad-version.oct", "wrong-type.oct", "bad-xml.oct", "dtd.oct", "conflicting-dimensions.oct",
        "overflow.oct", "raw-only.oct", "duplicate.oct", "truncated.oct", "unsupported-compression.oct"}) {
        ThorlabsOct::Metadata metadata;
        QString error;
        require(!ThorlabsOct::inspect(fixture(name), metadata, error), name);
        require(!error.isEmpty(), "unsupported input has actionable error");
    }
    for (const char* name : {"crc-error.oct", "nan.oct"}) {
        ThorlabsOct::Metadata metadata;
        QString error;
        vtkSmartPointer<vtkImageData> image;
        require(!ThorlabsOct::read(fixture(name), image, metadata, error), name);
        require(!image && !error.isEmpty(), "failed read publishes no partial volume");
    }
    ThorlabsOct::Metadata metadata;
    QString error;
    vtkSmartPointer<vtkImageData> image;
    require(!ThorlabsOct::read(fixture("stored.oct"), image, metadata, error, {}, 1024), "memory guard before allocation");
    require(error.contains("Insufficient memory") && !image, "memory failure is actionable and atomic");
    require(ThorlabsOct::inspect(fixture("unknown-calibration.oct"), metadata, error), qPrintable(error));
    require(metadata.calibratedAxes == 0, "unknown spacing not invented from optical Image metadata");
    require(ThorlabsOct::read(fixture("two-dimensional.oct"), image, metadata, error), qPrintable(error));
    require(image->GetDimensions()[1] == 1, "single B-scan dimension preserved");
    require(ThorlabsOct::read(fixture("large-deflate.oct"), image, metadata, error), qPrintable(error));
    near(image->GetScalarComponentAsDouble(256, 2, 82, 0), -20 + 40 + 4*256 + 82*0.25,
        "deflate block boundaries and last voxel preserved");
}

void loaderAndPipeline()
{
    OpenData dialog;
    require(QMetaObject::invokeMethod(&dialog, "fileAttributes", Qt::DirectConnection,
        Q_ARG(QString, fixture("stored.oct")), Q_ARG(QString, QString())), "OCT dialog dispatch");
    require(dialog.isValidData() && dialog.getDataFormat() == DATA_OCT && dialog.getBitsize() == BIT32FLOAT,
        "OCT recognized with correct bit-depth metadata");
    require(dialog.getWidth() == 3 && dialog.getHeight() == 2 && dialog.getDepth() == 4, "dialog dimensions");
    require(!dialog.findChild<QComboBox*>("comboBox_bitsize")->isEnabled(), "OCT scalar type is read-only");
    loading worker(&dialog);
    int successes = 0, failures = 0, finished = 0;
    vtkSmartPointer<vtkImageData> image;
    unsigned int mask = 0;
    double lo = 0, hi = 0;
    QObject::connect(&worker, &loading::dataLoaded, [&](vtkImageData* data, unsigned int axes, double minimum, double maximum) {
        ++successes; image.TakeReference(data); mask = axes; lo = minimum; hi = maximum;
    });
    QObject::connect(&worker, &loading::failed, [&](const QString&) { ++failures; });
    QObject::connect(&worker, &loading::finished, [&] { ++finished; });
    worker.loadData();
    require(successes == 1 && failures == 0 && finished == 1 && mask == 7, "production worker transfers one calibrated volume");
    near(lo, -20, "negative dB minimum retained");
    near(hi, 8.75, "fractional dB maximum retained");
    DocumentModel model;
    auto& data = model.create();
    data.image = image;
    data.dataFormat = DATA_OCT;
    data.zeroIsTransparent = false;
    data.minValue = data.currentMinThreshold = -20;
    data.maxValue = data.currentMaxThreshold = 9;
    data.VOI[1] = 3; data.VOI[3] = 2; data.VOI[5] = 4;
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    auto window = vtkSmartPointer<vtkRenderWindow>::New();
    window->SetOffScreenRendering(1); window->AddRenderer(renderer);
    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(window);
    VolumePipeline pipeline; Settings settings;
    const auto update = [&] {
        data.appearanceDirty = data.dataPipelineDirty = true;
        pipeline.update(data, settings, renderer, interactor);
    };
    update();
    near(data.volume->GetProperty()->GetScalarOpacityUnitDistance(), 1, "OCT MaxIP keeps its opacity distance");
    data.blendMode = 1;
    update();
    near(data.volume->GetProperty()->GetScalarOpacityUnitDistance(), 0.1, "OCT composite opacity is per voxel, not per mm");
    image->SetSpacing(0.003, 0.004, 0.0025);
    update();
    near(data.volume->GetProperty()->GetScalarOpacityUnitDistance(), 0.0025, "anisotropic OCT uses smallest voxel spacing");
    data.VOI[1] = 2;
    update();
    near(data.volume->GetProperty()->GetScalarOpacityUnitDistance(), 0.0025, "crop must not change OCT opacity reference");
    data.VOI[1] = 3;
    for (int blend : {0, 2, 3, 1}) {
        data.blendMode = blend;
        update();
        near(data.volume->GetProperty()->GetScalarOpacityUnitDistance(), blend == 1 ? 0.0025 : 1,
            "mode switches restore the correct opacity distance");
    }
    data.dataFormat = DATA_TIFF;
    update();
    near(data.volume->GetProperty()->GetScalarOpacityUnitDistance(), 1, "calibrated TIFF rendering is unchanged");
    data.dataFormat = DATA_OCT;
    update();
    near(data.opacityFun->GetValue(0), 20.0/29.0, "zero dB is not a transparent hole");
    near(data.opacityFun->GetValue(-10), 10.0/29.0, "negative dB keeps smooth opacity");
    data.currentMinThreshold = -5;
    update();
    near(data.threshold->GetOutput()->GetScalarComponentAsDouble(0, 0, 0, 0), -21, "rejected dB uses out-of-range sentinel");
    near(data.opacityFun->GetValue(-21), 0, "dB sentinel is fully transparent");
    near(data.threshold->GetOutput()->GetScalarComponentAsDouble(0, 1, 0, 0), 0, "valid 0 dB preserved");
    double black[3]; data.colorFun->GetColor(-21, black);
    near(black[0] + black[1] + black[2], 0, "plane rejects out-of-range voxels to black");
    data.smoothOpacity = false; update();
    near(data.opacityFun->GetValue(0), 1, "uniform mode includes zero dB");
    require(QMetaObject::invokeMethod(&dialog, "fileAttributes", Qt::DirectConnection,
        Q_ARG(QString, fixture("raw-only.oct")), Q_ARG(QString, QString())), "unsupported OCT dialog dispatch");
    require(!dialog.isValidData(), "unsupported file cannot reuse previous valid data");
}
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setOrganizationName("OCTview3R-regression"); app.setApplicationName("OctImportRegression");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir::currentPath() + "/tmp/oct-regression");
    try {
        fixtures(); loaderAndPipeline();
        std::printf("PASS: %d OCT archive, calibration, float, memory, loader and pipeline checks.\n", checks);
        return 0;
    } catch (const std::exception& failure) {
        std::printf("FAIL after %d checks: %s\n", checks, failure.what()); return 1;
    }
}
