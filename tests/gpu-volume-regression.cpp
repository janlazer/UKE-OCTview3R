// Render the production pipeline on a real OpenGL context. Filter-only tests
// cannot detect invalid generated GLSL. No scientific data is bundled here.
#include "../OCTview3R/documentModel.h"
#include "../OCTview3R/viewerData.h"
#include "../OCTview3R/volumePipeline.h"
#include "../OCTview3R/loading.h"
#include "../OCTview3R/opendata.h"
#include <QApplication>
#include <QDir>
#include <QSettings>
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkImageMapToColors.h>
#include <vtkImagePlaneWidget.h>
#include <vtkObjectFactory.h>
#include <vtkOutputWindow.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPNGWriter.h>
#include <vtkVersion.h>
#include <vtkWindowToImageFilter.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace {
int checks = 0;
void require(bool ok, const char* message)
{
    ++checks;
    if (!ok) throw std::runtime_error(message);
}
class RenderLog : public vtkOutputWindow
{
public:
    static RenderLog* New();
    vtkTypeMacro(RenderLog, vtkOutputWindow);
    void DisplayText(const char* text) override { std::fputs(text, stderr); }
    void DisplayErrorText(const char* text) override { ++errors; DisplayText(text); }
    int errors = 0;
};
vtkStandardNewMacro(RenderLog);

vtkSmartPointer<vtkImageData> sample(int components)
{
    auto image = vtkSmartPointer<vtkImageData>::New();
    image->SetDimensions(32, 32, 32);
    image->AllocateScalars(VTK_UNSIGNED_CHAR, components);
    for (int z = 0; z < 32; ++z)
        for (int y = 0; y < 32; ++y)
            for (int x = 0; x < 32; ++x) {
                const bool inside = x >= 8 && x < 24 && y >= 8 && y < 24 && z >= 8 && z < 24;
                for (int c = 0; c < components; ++c)
                    image->SetScalarComponentFromDouble(x, y, z, c,
                        c == 3 ? 255 : inside ? (c == 1 ? 190 : 70) : 0);
            }
    return image;
}

struct Scene
{
    DocumentModel documents;
    ImageData& data = documents.create();
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkRenderWindow> window = vtkSmartPointer<vtkRenderWindow>::New();
    vtkSmartPointer<vtkRenderWindowInteractor> interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
    VolumePipeline pipeline;
    Settings settings;
    RenderLog* log;

    Scene(vtkImageData* image, RenderLog* output) : log(output)
    {
        window->SetOffScreenRendering(1);
        window->SetSize(160, 160);
        window->SetMultiSamples(0);
        window->AddRenderer(renderer);
        interactor->SetRenderWindow(window);
        renderer->SetBackground(0.10, 0.20, 0.30);
        data.image = image;
        const int* extent = image->GetExtent();
        for (int axis = 0; axis < 3; ++axis) {
            data.VOI[2 * axis] = extent[2 * axis];
            data.VOI[2 * axis + 1] = extent[2 * axis + 1] + 1;
        }
        data.isVolume = data.fileLoaded = data.showObject = true;
        data.renderRgb = image->GetNumberOfScalarComponents() >= 3;
        // Fail instead of silently testing the CPU fallback.
        data.volumeMapper->SetRequestedRenderModeToGPU();
    }
    ~Scene()
    {
        data.planeWidget->Off();
        data.planeWidget->SetInteractor(nullptr);
        renderer->RemoveAllViewProps();
        window->Finalize();
    }
    void update()
    {
        data.appearanceDirty = data.dataPipelineDirty = data.planeDirty = true;
        pipeline.update(data, settings, renderer, interactor);
    }
    double render()
    {
        update();
        window->Render();
        require(log->errors == 0, "VTK reported a rendering/shader error");
        require(data.volumeMapper->GetLastUsedRenderMode() == vtkSmartVolumeMapper::GPURenderMode,
            "test must use the GPU volume mapper");
        auto capture = vtkSmartPointer<vtkWindowToImageFilter>::New();
        capture->SetInput(window);
        capture->SetInputBufferTypeToRGB();
        capture->ReadFrontBufferOff();
        capture->ShouldRerenderOff();
        capture->Update();
        auto pixels = capture->GetOutput();
        const int* size = pixels->GetDimensions();
        double contrast = 0;
        for (int y = 0; y < size[1]; ++y)
            for (int x = 0; x < size[0]; ++x)
                for (int c = 0; c < 3; ++c)
                    contrast += std::abs(pixels->GetScalarComponentAsDouble(x, y, 0, c) -
                        pixels->GetScalarComponentAsDouble(0, 0, 0, c));
        require(log->errors == 0, "VTK reported an image-capture error");
        return contrast / (size[0] * size[1] * 3);
    }
    void frame()
    {
        update();
        renderer->ResetCamera();
        renderer->GetActiveCamera()->Azimuth(20);
        renderer->GetActiveCamera()->Elevation(15);
        renderer->ResetCameraClippingRange();
    }
    double centerIntensity()
    {
        auto capture = vtkSmartPointer<vtkWindowToImageFilter>::New();
        capture->SetInput(window);
        capture->SetInputBufferTypeToRGB();
        capture->ReadFrontBufferOff();
        capture->ShouldRerenderOff();
        capture->Update();
        auto pixels = capture->GetOutput();
        const int* size = pixels->GetDimensions();
        double sum = 0;
        for (int y = size[1] / 2 - 4; y < size[1] / 2 + 4; ++y)
            for (int x = size[0] / 2 - 4; x < size[0] / 2 + 4; ++x)
                for (int c = 0; c < 3; ++c)
                    sum += pixels->GetScalarComponentAsDouble(x, y, 0, c);
        return sum / (64 * 3 * 255);
    }
};

void syntheticTests(RenderLog* log, int components)
{
    Scene scene(sample(components), log);
    scene.data.blendMode = 1;
    scene.frame();
    require(scene.data.volume->GetProperty()->GetScalarOpacityUnitDistance() == 1.0,
        "legacy grayscale/RGB opacity distance must remain unchanged");
#if VTK_MAJOR_VERSION == 8 && VTK_MINOR_VERSION == 2
    auto property = scene.data.volume->GetProperty();
    require(property->HasGradientOpacity(), "VTK 8.2 needs a named gradient sampler before first render");
    require(property->GetDisableGradientOpacity() != 0, "gradient opacity must stay disabled");
    for (double value : { 0.0, 128.0, 65535.0 })
        require(property->GetStoredGradientOpacity()->GetValue(value) == 1,
            "compatibility table must be neutral at all gradient magnitudes");
#endif
    require(scene.render() > 1, "fresh composite volume must be visible");
    // Exercise every supported shader branch, then repeat mode changes on the
    // same mapper so cached transfer tables cannot hide a regression.
    for (int pass = 0; pass < 3; ++pass) {
        scene.data.renderRgb = components >= 3 && pass != 1;
        for (int blend = 0; blend < 4; ++blend) {
            scene.data.blendMode = blend;
            const double contrast = scene.render();
            // Scalar MaxIP applies the soft alpha ramp once; its small cube is
            // intentionally fainter than opaque RGB, but must not disappear.
            if (blend <= 1) require(contrast > 0.1, "MaxIP/composite volume must remain visible");
        }
    }
    scene.data.blendMode = 1;
    scene.data.objectOpacity = 0;
    require(scene.render() < 0.1, "zero opacity must reveal the background");
    scene.data.objectOpacity = 1;
    scene.data.currentMinThreshold = 240;
    require(scene.render() < 0.1, "fully rejected voxels must be transparent");
    scene.data.currentMinThreshold = 0;
    require(scene.render() > 1, "restoring thresholds must restore visibility");
    scene.data.showPlane = scene.data.changePlaneInput = scene.data.orientChanged = true;
    scene.render();
    scene.data.colorMap->Update();
    require(scene.data.colorMap->GetOutput()->GetNumberOfScalarComponents() == (components >= 3 ? 3 : 4),
        "RGB plane passes three opaque channels, grayscale plane maps to RGBA");
    std::printf("PASS: synthetic %d-component GPU rendering and transitions.\n", components);
}

void octCompositeTest(RenderLog* log)
{
    double previousIntensity = -1;
    // Identical signed dB voxels in pixel-sized and micrometre-sized coordinates.
    // A mere non-empty-image check misses a nearly transparent composite volume.
    for (double spacing : {1.0, 0.003}) {
        auto image = vtkSmartPointer<vtkImageData>::New();
        image->SetDimensions(32, 32, 32);
        image->SetSpacing(spacing, spacing, spacing);
        image->AllocateScalars(VTK_FLOAT, 1);
        for (int z = 0; z < 32; ++z)
            for (int y = 0; y < 32; ++y)
                for (int x = 0; x < 32; ++x)
                    image->SetScalarComponentFromDouble(x, y, z, 0,
                        x >= 8 && x < 24 && y >= 8 && y < 24 && z >= 8 && z < 24 ? 0.0 : -60.0);
        Scene scene(image, log);
        scene.data.dataFormat = DATA_OCT;
        scene.data.zeroIsTransparent = false;
        scene.data.minValue = scene.data.currentMinThreshold = -60;
        scene.data.maxValue = scene.data.currentMaxThreshold = 20;
        scene.data.blendMode = 1;
        scene.frame();
        scene.renderer->SetBackground(0, 0, 0);
        scene.render();
        const double blackBackground = scene.centerIntensity();
        scene.renderer->SetBackground(1, 1, 1);
        scene.render();
        const double transmission = scene.centerIntensity() - blackBackground;
        std::printf("OCT composite: spacing %.6g, center %.4f, background transmission %.4f\n",
            spacing, blackBackground, transmission);
        require(transmission < 0.05, "dense OCT composite must occlude the background at micrometre spacing");
        require(blackBackground > 0.5, "OCT composite must retain visible grayscale, not just opaque black");
        if (previousIntensity >= 0)
            require(std::abs(blackBackground - previousIntensity) < 0.03,
                "OCT composite appearance must not depend on coordinate-unit scale");
        previousIntensity = blackBackground;
        scene.data.objectOpacity = 0.01;
        scene.render();
        require(scene.centerIntensity() > blackBackground + 0.1, "OCT opacity slider must still reveal the background");
        scene.data.objectOpacity = 0;
        scene.render();
        require(scene.centerIntensity() > 0.99, "zero OCT opacity must fully reveal the white background");
        scene.data.objectOpacity = 1;
        scene.data.currentMinThreshold = 1;
        scene.render();
        require(scene.centerIntensity() > 0.99, "rejected zero dB must remain transparent");
    }
    std::puts("PASS: OCT composite density, coordinate scale, opacity and threshold masking.");
}

void localVolumeTest(const QString& path, RenderLog* log)
{
    OpenData dialog;
    require(QMetaObject::invokeMethod(&dialog, "fileAttributes", Qt::DirectConnection,
        Q_ARG(QString, path), Q_ARG(QString, QString())), "initialize the volume import dialog");
    loading reader(&dialog);
    vtkSmartPointer<vtkImageData> image;
    unsigned int calibration = 0;
    double minimum = 0, maximum = 255;
    QString error;
    QObject::connect(&reader, &loading::dataLoaded,
        [&](vtkImageData* loaded, unsigned int mask, double lo, double hi) {
            image.TakeReference(loaded); calibration = mask; minimum = lo; maximum = hi;
        });
    QObject::connect(&reader, &loading::failed, [&](const QString& message) { error = message; });
    reader.loadData();
    if (!error.isEmpty()) throw std::runtime_error(error.toStdString());
    require(image && image->GetNumberOfPoints() > 0, "local volume must load via production reader");
    const int* dimensions = image->GetDimensions();
    const double* spacing = image->GetSpacing();
    std::printf("Volume: %d x %d x %d, %d components, spacing %.9g %.9g %.9g, calibrated axes %u\n",
        dimensions[0], dimensions[1], dimensions[2], image->GetNumberOfScalarComponents(),
        spacing[0], spacing[1], spacing[2], calibration);
    Scene scene(image, log);
    scene.window->SetSize(640, 640);
    scene.data.dataFormat = dialog.getDataFormat();
    scene.data.zeroIsTransparent = scene.data.dataFormat != DATA_OCT;
    scene.data.minValue = scene.data.currentMinThreshold = static_cast<int>(std::floor(minimum));
    scene.data.maxValue = scene.data.currentMaxThreshold = static_cast<int>(std::ceil(maximum));
    scene.frame();
    for (int pass = 0; pass < 3; ++pass) {
        scene.data.renderRgb = image->GetNumberOfScalarComponents() >= 3 && pass != 1;
        for (int blend = 0; blend < 2; ++blend) {
            scene.data.blendMode = blend;
            require(scene.render() > 0.5, "local volume must produce a non-empty image in RGB/grayscale");
            if (pass == 0) {
                // Local-only visual QA. The optional input may be research data;
                // these images belong in ignored tmp storage, never paper/assets.
                const QString directory = QDir::currentPath() + "/tmp/gpu-volume-regression";
                require(QDir().mkpath(directory), "create local-only render QA directory");
                auto capture = vtkSmartPointer<vtkWindowToImageFilter>::New();
                capture->SetInput(scene.window); capture->ReadFrontBufferOff();
                capture->ShouldRerenderOff(); capture->Update();
                auto writer = vtkSmartPointer<vtkPNGWriter>::New();
                writer->SetFileName((directory + (blend == 0 ? "/local-maxip.png" : "/local-composite.png")).toLocal8Bit().constData());
                writer->SetInputConnection(capture->GetOutputPort()); writer->Write();
                require(log->errors == 0, "write local render QA image");
            }
        }
    }
    std::puts("PASS: local volume loader, colour modes, MaxIP and composite GPU rendering.");
}
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
        QDir::currentPath() + "/tmp/gpu-volume-regression");
    app.setOrganizationName("OCTview3R-regression");
    app.setApplicationName("GpuVolumeRegression");
    auto log = vtkSmartPointer<RenderLog>::New();
    vtkOutputWindow::SetInstance(log);
    try {
        // Start with dependent RGBA before any scalar render initializes VTK defaults.
        syntheticTests(log, 4);
        syntheticTests(log, 3);
        syntheticTests(log, 1);
        octCompositeTest(log);
        if (app.arguments().size() > 1) localVolumeTest(app.arguments().at(1), log);
        require(log->errors == 0, "VTK errors during cleanup");
        std::printf("PASS: %d GPU volume checks.\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::printf("FAIL after %d checks: %s\n", checks, error.what());
        return 1;
    }
}
