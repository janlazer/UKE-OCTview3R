// Exercises the production pipeline and Qt controls, using only synthetic data.
#include "../OCTview3R/OCTview3R.h"
#include "../OCTview3R/volumePipeline.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QSettings>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTimer>
#include <vtkColorTransferFunction.h>
#include <vtkImageAppendComponents.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageThreshold.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace {
int checks = 0;
void require(bool condition, const char* label)
{
    ++checks;
    if (!condition) throw std::runtime_error(label);
}
void near(double actual, double expected, const char* label)
{
    if (std::abs(actual - expected) > 1e-6) {
        std::printf("%s: actual %.9f, expected %.9f\n", label, actual, expected);
        throw std::runtime_error(label);
    }
    ++checks;
}
vtkSmartPointer<vtkImageData> gradient(int type = VTK_UNSIGNED_CHAR, int components = 1)
{
    auto image = vtkSmartPointer<vtkImageData>::New();
    image->SetDimensions(256, 2, 2);
    image->AllocateScalars(type, components);
    for (int z = 0; z < 2; ++z)
        for (int y = 0; y < 2; ++y)
            for (int x = 0; x < 256; ++x)
                for (int c = 0; c < components; ++c)
                    image->SetScalarComponentFromDouble(x, y, z, c,
                        type == VTK_UNSIGNED_SHORT ? x * 257 : x);
    return image;
}
double gray(ImageData& data, double scalar)
{
    double color[3] = {};
    data.colorFun->GetColor(scalar, color);
    return color[0];
}
void pipelineTests()
{
    DocumentModel documents;
    ImageData& data = documents.create();
    data.image = gradient();
    data.VOI[1] = 256; data.VOI[3] = 2; data.VOI[5] = 2;
    data.isVolume = data.fileLoaded = true;
    auto renderer = vtkSmartPointer<vtkRenderer>::New();
    auto window = vtkSmartPointer<vtkRenderWindow>::New();
    window->SetOffScreenRendering(1);
    window->AddRenderer(renderer);
    auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    interactor->SetRenderWindow(window);
    VolumePipeline pipeline;
    Settings settings;
    const auto update = [&] {
        data.appearanceDirty = data.dataPipelineDirty = true;
        pipeline.update(data, settings, renderer, interactor);
    };
    update();
    near(data.opacityFun->GetValue(0), 0, "source black is transparent");
    near(data.opacityFun->GetValue(128), 128.0 / 255, "default soft opacity");
    near(data.opacityFun->GetValue(255), 1, "maximum opacity");

    data.currentMinThreshold = 100;
    data.currentMaxThreshold = 200;
    update();
    for (int value = 100; value <= 200; ++value) {
        near(data.opacityFun->GetValue(value), (value - 100.0) / 100, "linear soft ramp");
        near(gray(data, value), (value - 100.0) / 100, "original palette autoscale");
    }
    near(data.threshold->GetOutput()->GetScalarComponentAsDouble(99, 0, 0, 0), 0, "below Min masked");
    near(data.threshold->GetOutput()->GetScalarComponentAsDouble(150, 0, 0, 0), 150, "accepted intensity preserved");
    near(data.threshold->GetOutput()->GetScalarComponentAsDouble(201, 0, 0, 0), 0, "above Max masked");
    near(data.opacityFun->GetValue(0), 0, "threshold mask transparent");
    near(data.opacityFun->GetValue(201), 0, "above Max has zero opacity");

    data.adjustColormap = false;
    update();
    near(gray(data, 150), 150.0 / 255, "fixed palette uses source range");
    near(data.opacityFun->GetValue(150), 0.5, "palette does not affect opacity");
    data.adjustColormap = true;
    data.autoWindow = false;
    data.windowWidth = 128;
    data.windowLevel = 96;
    update();
    const double expected = (127.5 - 95.5) / 127 + 0.5;
    near(gray(data, 150), expected, "manual window works on autoscaled palette");
    near(data.opacityFun->GetValue(150), 0.5, "window does not affect opacity");
    data.adjustColormap = false;
    update();
    near(gray(data, 150), (150.0 - 95.5) / 127 + 0.5, "manual raw-intensity window without autoscale");
    near(data.opacityFun->GetValue(150), 0.5, "manual raw window preserves opacity");
    data.adjustColormap = true;
    data.currentMinThreshold = 120;
    update();
    require(data.windowWidth == 128 && data.windowLevel == 96, "threshold preserves manual window");
    near(data.opacityFun->GetValue(160), 0.5, "ramp follows changed threshold");
    data.autoWindow = true;
    update();
    near(gray(data, 160), 0.5, "auto window restores neutral mapping");

    data.colormapName = "Black Body";
    data.invertColormap = true;
    update();
    double inverted[3] = {};
    data.colorFun->GetColor(180, inverted); // palette position 0.75 -> inverted 0.25
    near(inverted[0], 1, "inverted palette red stop");
    near(inverted[1], 0, "inverted palette green stop");
    near(inverted[2], 0, "inverted palette blue stop");
    near(data.opacityFun->GetValue(160), 0.5, "palette inversion preserves opacity");
    data.colormapName = "Greyscale";
    data.invertColormap = false;
    for (int blend = 0; blend < 4; ++blend) {
        data.blendMode = blend;
        update();
        near(data.opacityFun->GetValue(160), 0.5, "soft opacity available in all blend modes");
    }
    data.objectOpacity = 0;
    update();
    near(data.opacityFun->GetValue(160), 0, "zero object opacity hides accepted values");

    data.smoothOpacity = false;
    data.objectOpacity = 0.6;
    update();
    near(data.opacityFun->GetValue(121), 0.6, "uniform mode retains hard-cutoff option");
    near(data.opacityFun->GetValue(160), 0.6, "uniform accepted opacity");
    near(data.opacityFun->GetValue(0), 0, "uniform mode keeps black transparent");

    // Use the real plane setup. Its RGBA output must not inherit volume alpha.
    data.showPlane = data.changePlaneInput = true;
    update();
    data.colorMap->SetInputConnection(data.threshold->GetOutputPort());
    data.colorMap->Update();
    near(data.colorMap->GetOutput()->GetScalarComponentAsDouble(0, 0, 0, 0), 0, "plane black color");
    near(data.colorMap->GetOutput()->GetScalarComponentAsDouble(0, 0, 0, 3), 255, "plane black remains opaque");
    data.showPlane = false;
    data.changePlaneInput = true;
    update();

    data.smoothOpacity = true;
    data.currentMinThreshold = data.currentMaxThreshold = 150;
    update();
    near(data.opacityFun->GetValue(150), 0.6, "single-value interval is usable");
    data.currentMinThreshold = data.currentMaxThreshold = 0;
    update();
    near(data.opacityFun->GetValue(0), 0, "zero-only interval stays invisible");
    data.image = gradient(VTK_UNSIGNED_SHORT);
    data.maxValue = 65535;
    data.currentMinThreshold = 10000;
    data.currentMaxThreshold = 60000;
    data.objectOpacity = 1;
    update();
    near(data.opacityFun->GetValue(35000), 0.5, "16-bit smooth ramp");
    near(gray(data, 35000), 0.5, "16-bit palette autoscale");

    data.image = gradient(VTK_UNSIGNED_CHAR, 3);
    data.maxValue = 255;
    data.currentMinThreshold = 100;
    data.currentMaxThreshold = 200;
    data.renderRgb = true;
    update();
    data.rgbaVolume->Update();
    auto* rgba = data.rgbaVolume->GetOutput();
    require(rgba->GetNumberOfScalarComponents() == 4, "RGB mapper receives RGBA");
    near(rgba->GetScalarComponentAsDouble(0, 0, 0, 3), 0, "RGB black alpha");
    near(rgba->GetScalarComponentAsDouble(99, 0, 0, 3), 0, "RGB below Min alpha");
    near(rgba->GetScalarComponentAsDouble(150, 0, 0, 3), 255, "RGB accepted alpha");
    near(rgba->GetScalarComponentAsDouble(201, 0, 0, 3), 0, "RGB above Max alpha");
    near(data.opacityFun->GetValue(255), 1, "RGB remains uniformly opaque");
    near(rgba->GetScalarComponentAsDouble(150, 0, 0, 0), 150, "RGB full-source window preserves channel");
    data.autoWindow = false;
    data.windowWidth = 128;
    data.windowLevel = 128;
    update();
    data.rgbaVolume->Update();
    near(data.rgbaVolume->GetOutput()->GetScalarComponentAsDouble(150, 0, 0, 3), 255, "RGB manual window preserves accepted alpha");
    near(data.rgbaVolume->GetOutput()->GetScalarComponentAsDouble(0, 0, 0, 3), 0, "RGB manual window preserves black alpha");
    data.renderRgb = false;
    update();
    near(data.opacityFun->GetValue(150), 0.5, "RGB to grayscale restores soft ramp");
    require(data.threshold->GetOutput()->GetNumberOfScalarComponents() == 1, "RGB to grayscale scalar output");
}

class TestWindow : public OCTview3R {
public:
    using OCTview3R::slotDataFileDialogClosed;
    using OCTview3R::slotSetImageData;
    using OCTview3R::slotSetThreshold;
};

void uiTests(QApplication& app, const QString& output)
{
    TestWindow window;
    window.setAttribute(Qt::WA_DontShowOnScreen);
    window.resize(1500, 1150);
    window.show();
    auto* loader = window.findChild<OpenData*>();
    require(loader != nullptr, "volume import dialog exists");
    const QString fixture = output + "/gradient.raw";
    QFile file(fixture);
    require(file.open(QIODevice::WriteOnly), "create synthetic fixture");
    file.write(QByteArray(1024, '\0'));
    file.close();
    // Select a generated fixture through the application's actual import dialog.
    // No private state or production test hooks are needed.
    QTimer::singleShot(0, [&] {
        auto* dialog = loader->findChild<QFileDialog*>();
        require(dialog != nullptr, "file selection dialog exists");
        dialog->selectFile(fixture);
        QMetaObject::invokeMethod(dialog, "accept", Qt::DirectConnection);
    });
    loader->openFile();
    require(loader->isValidData(), "synthetic selection accepted");
    auto image = gradient();
    image->Register(nullptr); // load callback consumes one reference
    window.slotDataFileDialogClosed(image, 0, 0, 255);
    auto* palette = window.findChild<QCheckBox*>("checkBox_adjustColormap");
    auto* automatic = window.findChild<QCheckBox*>("checkBox_autoWindow");
    auto* smooth = window.findChild<QCheckBox*>("checkBox_smoothOpacity");
    auto* width = window.findChild<QSpinBox*>("windowWidthSpinBox");
    auto* level = window.findChild<QSpinBox*>("windowLevelSpinBox");
    auto* minimum = window.findChild<QSpinBox*>("minThresholdSpinBox");
    auto* maximum = window.findChild<QSpinBox*>("maxThresholdSpinBox");
    auto* tabs = window.findChild<QTabWidget*>("tabWidget");
    require(palette && automatic && smooth && width && level && minimum && maximum && tabs,
        "all Designer controls exist");
    require(palette->isChecked() && automatic->isChecked() && smooth->isChecked(), "new scalar defaults");
    width->setValue(128);
    level->setValue(96);
    require(!automatic->isChecked() && palette->isChecked(), "manual W/L disables only Auto window");
    minimum->setValue(100);
    maximum->setValue(200);
    window.slotSetThreshold();
    require(width->value() == 128 && level->value() == 96, "UI threshold preserves manual W/L");
    palette->click();
    require(width->value() == 128 && level->value() == 96, "palette toggle preserves manual W/L");
    automatic->click();
    require(width->value() == 256 && level->value() == 128, "Auto window restores source range");
    require(!palette->isChecked(), "Auto window leaves palette toggle alone");
    smooth->click();
    require(!smooth->isChecked(), "uniform opacity selectable");
    auto second = gradient(VTK_UNSIGNED_SHORT);
    second->Register(nullptr);
    window.slotDataFileDialogClosed(second, 0, 0, 65535);
    require(palette->isChecked() && automatic->isChecked() && smooth->isChecked(), "second dataset independent defaults");
    require(width->value() == 65536, "16-bit UI window range");
    tabs->setCurrentIndex(0);
    require(!palette->isChecked() && automatic->isChecked() && !smooth->isChecked(), "dataset settings restored");
    require(minimum->value() == 100 && maximum->value() == 200, "dataset thresholds restored");
    minimum->setValue(220);
    window.slotSetThreshold();
    require(minimum->value() <= maximum->value(), "spin-box interval cannot invert");
    minimum->setValue(100);
    window.slotSetThreshold();
    palette->click();
    smooth->click();

    app.processEvents();
    require(window.findChild<QGroupBox*>("groupBox_colorMapping")->grab().save(output + "/color-controls.png"),
        "save actual Designer color controls");
    require(window.findChild<QGroupBox*>("groupBox_threshold")->grab().save(output + "/threshold-controls.png"),
        "save actual Designer threshold controls");

    auto rgb = gradient(VTK_UNSIGNED_CHAR, 3);
    rgb->Register(nullptr);
    window.slotDataFileDialogClosed(rgb, 0, 0, 255);
    require(!palette->isEnabled() && !smooth->isEnabled() && automatic->isEnabled(), "RGB controls have accurate availability");
    tabs->setCurrentIndex(0);
    require(palette->isEnabled() && smooth->isEnabled(), "grayscale controls re-enabled");
}
}

int main(int argc, char** argv)
{
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    QApplication app(argc, argv);
    const QString output = QDir::currentPath() + "/tmp/threshold-regression";
    QDir().mkpath(output);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, output);
    app.setOrganizationName("OCTview3R-regression");
    app.setApplicationName("ThresholdRegression");
    try {
        pipelineTests();
        uiTests(app, output);
        std::printf("PASS: %d threshold, palette, window, plane, RGB and UI checks.\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::printf("FAIL after %d checks: %s\n", checks, error.what());
        return 1;
    }
}
