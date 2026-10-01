// Exercise actual Designer forms and settings migration without user preferences.
#include "../OCTview3R/OCTview3R.h"
#include "../OCTview3R/aboutdialog.h"
#include "../OCTview3R/viewerController.h"
#include "ui_OCTview3R.h"
#include "ui_aboutdialog.h"
#include "ui_opendata.h"
#include "ui_openpoly.h"
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QToolBar>
#include <QVTKOpenGLWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkOutputWindow.h>
#include <vtkImageData.h>
#include <vtkImagePlaneWidget.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace {
int checks = 0;
class RecordingVtkOutput : public vtkOutputWindow {
public:
    static RecordingVtkOutput* New() { return new RecordingVtkOutput; }
    vtkTypeMacro(RecordingVtkOutput, vtkOutputWindow);
    void DisplayText(const char* text) override { messages += text; }
    std::string messages;
};
void require(bool ok, const char* description)
{
    if (!ok) throw std::runtime_error(description);
    ++checks;
}
void sceneShutdownTests()
{
    auto vtkOutput = vtkSmartPointer<RecordingVtkOutput>::New();
    vtkOutputWindow::SetInstance(vtkOutput);
    { ViewerController unused; }
    require(vtkOutput->messages.empty(), "unused controller teardown emits no VTK diagnostics");
    for (int scenario = 0; scenario < 4; ++scenario) {
        auto renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
        renderWindow->SetOffScreenRendering(1);
        renderWindow->SetSize(160, 160);
        auto interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
        interactor->SetRenderWindow(renderWindow);
        Settings settings;
        settings.showScalarBar = settings.showOrientAxes = scenario == 3;
        DocumentModel documents;
        auto controller = std::make_unique<ViewerController>();
        controller->initialize(renderWindow, settings);
        interactor->Initialize();
        interactor->SetEventPosition(80, 80);
        interactor->SetLastEventPosition(80, 80);
        if (scenario != 0) {
            auto& data = documents.create();
            data.fileLoaded = data.showObject = true;
            if (scenario == 1) {
                data.isPolyData = true;
                auto sphere = vtkSmartPointer<vtkSphereSource>::New();
                sphere->Update();
                data.poly = sphere->GetOutput();
                data.poly->GetBounds(data.VOI);
                data.poly->GetBounds(data.sourceVOI);
            } else {
                data.isVolume = true;
                data.image->SetDimensions(4, 4, 4);
                data.image->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
                for (int z = 0; z < 4; ++z)
                    for (int y = 0; y < 4; ++y)
                        for (int x = 0; x < 4; ++x)
                            data.image->SetScalarComponentFromDouble(x, y, z, 0, 64 + x * 32);
                data.width = data.height = data.depth = 4;
                data.VOI[1] = data.VOI[3] = data.VOI[5] = 4;
                data.showPlane = data.orientChanged = data.changePlaneInput = scenario == 3;
                data.planeWidget->SetCurrentRenderer(controller->renderer());
            }
            controller->refresh(documents, &data, settings, true);
            if (scenario == 3) {
                require(data.planeWidget->GetEnabled() != 0, "shutdown fixture has an active plane");
                require(renderWindow->GetRenderers()->GetNumberOfItems() > 1, "shutdown fixture has an active orientation marker");
            }
        }
        int renders = 0;
        auto observer = vtkSmartPointer<vtkCallbackCommand>::New();
        observer->SetClientData(&renders);
        observer->SetCallback([](vtkObject*, unsigned long, void* count, void*) { ++*static_cast<int*>(count); });
        const auto tag = renderWindow->AddObserver(vtkCommand::StartEvent, observer);
        if (scenario == 1) {
            vtkOutput->messages.clear();
            controller->clear(documents);
            require(renders > 0 && interactor->GetEnableRender(), "ordinary scene clear still renders and permits future interaction");
            require(vtkOutput->messages.empty(), "ordinary PolyData removal does not disable an unassigned plane widget");
            controller->refresh(documents, documents.active(), settings);
            renders = 0;
        }
        vtkOutput->messages.clear();
        controller->shutdown(documents);
        controller->shutdown(documents); // closeEvent followed by the destructor
        controller->render();
        controller->refreshDecorations(documents, documents.active(), settings);
        require(controller->renderWindow() == nullptr && controller->interactor() == nullptr, "shutdown releases borrowed window connections");
        require(!interactor->GetEnableRender(), "shutdown blocks indirect widget renders");
        for (const auto& document : documents.documents()) {
            require(document->planeWidget->GetInteractor() == nullptr && !document->planeWidget->GetEnabled(), "shutdown detaches document plane widgets");
        }
        controller.reset();
        documents.clear();
        require(renders == 0, "scene teardown does not render with empty, poly, volume or decorated volume data");
        require(renderWindow->GetRenderers()->GetNumberOfItems() == 0, "shutdown removes scene and orientation renderers");
        if (!vtkOutput->messages.empty()) std::fprintf(stderr, "%s", vtkOutput->messages.c_str());
        require(vtkOutput->messages.empty(), "scene teardown emits no VTK diagnostics");
        renderWindow->RemoveObserver(tag);
        renderWindow->Finalize();
    }
}
void designerFormTests(QApplication& app, const QString& output)
{
    // No OCTview3R constructor, theme adapter or saved settings: exactly the
    // generated .ui setup used by Designer, under Qt's native application style.
    require(app.styleSheet().isEmpty(), "Designer checks start without application styling");
    QMainWindow preview;
    Ui_OCTview3R form;
    form.setupUi(&preview);
    preview.setAttribute(Qt::WA_DontShowOnScreen);
    preview.resize(1280, 900);
    preview.show();
    app.processEvents();
    std::printf("Designer dock: %d, button: %d, toolbar: %d\n", form.dockWidget_preferences->width(), form.pushButton_reset->height(), form.toolBar->height());
    std::printf("Designer preferences: viewport %d, minimum %d, scroll maximum %d\n", form.preferencesScrollArea->viewport()->width(), form.dockWidgetContents_3->minimumSizeHint().width(), form.preferencesScrollArea->horizontalScrollBar()->maximum());
    require(preview.grab().save(output + "/designer-main.png"), "save standalone Designer main form");
    require(preview.palette().color(QPalette::Window) == QColor(25, 33, 41), "main Designer form contains the dark palette");
    require(!preview.styleSheet().isEmpty(), "main Designer form contains the complete stylesheet");
    require(form.dockWidget_preferences->width() <= 490, "Designer default dock width matches the compact application layout");
    require(form.pushButton_reset->height() <= 28, "Designer buttons are compact without runtime styling");
    require(form.toolBar->height() <= 34, "Designer toolbar is compact without runtime styling");
    require(form.preferencesScrollArea->horizontalScrollBar()->maximum() == 0, "Designer preferences fit without horizontal scrolling");
    require(form.dockWidgetContents_3->palette().color(QPalette::Window) == QColor(25, 33, 41), "Designer scroll content stays dark across the dock boundary");
    require(form.metadataLabel->palette().color(QPalette::WindowText) == QColor(225, 235, 240), "Designer labels retain light text without an application palette");
    preview.resize(1920, 1080);
    app.processEvents();
    require(form.dockWidget_preferences->width() < form.dockWidget_visualization->width(), "wide Designer windows give most space to visualization");
    preview.hide();

    auto verifyDialog = [&](auto& ui, const QString& name) {
        QDialog dialog;
        ui.setupUi(&dialog);
        dialog.setAttribute(Qt::WA_DontShowOnScreen);
        dialog.show();
        app.processEvents();
        require(dialog.palette().color(QPalette::Window) == preview.palette().color(QPalette::Window), "standalone dialog form contains its own dark palette");
        require(!dialog.styleSheet().isEmpty(), "standalone dialog form contains its own Designer styling");
        require(dialog.height() >= dialog.minimumSizeHint().height(), "standalone dialog form fits its contents");
        require(dialog.grab().save(output + "/designer-" + name + ".png"), "save standalone Designer dialog");
    };
    Ui_AboutDialog about;
    Ui_OpenData data;
    Ui_OpenPoly poly;
    verifyDialog(about, "about");
    verifyDialog(data, "OpenData");
    verifyDialog(poly, "OpenPoly");
}
void swatchStyleTests(QApplication& app)
{
    class TestWindow : public OCTview3R {
    public:
        using OCTview3R::slotSetImageData;
    };
    DocumentModel fixture;
    auto& data = fixture.create();
    data.fileLoaded = data.isPolyData = true;
    data.fileName = "synthetic-mesh.vtk";
    auto sphere = vtkSmartPointer<vtkSphereSource>::New();
    sphere->Update();
    data.poly = sphere->GetOutput();
    data.poly->GetBounds(data.VOI);
    data.poly->GetBounds(data.sourceVOI);
    TestWindow window;
    window.setAttribute(Qt::WA_DontShowOnScreen);
    window.showNormal();
    auto* poly = window.findChild<QPushButton*>("pushButton_pickPolyColor");
    auto* volume = window.findChild<QPushButton*>("pushButton_pickVolumeColor");
    const QString polyStyle = poly->styleSheet();
    // Both declaration-only Designer styles and selector-based styles must work.
    volume->setStyleSheet("QPushButton { padding: 0; min-height: 18px; border: 1px solid #ff8800; }");
    const QString volumeStyle = volume->styleSheet();
    int lastStyleLength = 0;
    for (const auto color : { QColor("#40c7db"), QColor("#ee9966"), QColor("#55cc88") }) {
        data.polyColor = data.volumeColor = color;
        window.slotSetImageData(&data);
        app.processEvents();
        require(poly->styleSheet().startsWith(polyStyle), "dataset changes preserve the Designer swatch declarations");
        require(volume->styleSheet().startsWith(volumeStyle), "dataset changes preserve selector-based Designer styling");
        for (auto* swatch : {poly, volume}) {
            require(swatch->size() == QSize(22, 22), "dataset colour changes preserve compact Designer swatch sizes");
            const auto pixels = swatch->grab().toImage();
            require(pixels.pixelColor(pixels.width() / 2, pixels.height() / 2) == color, "swatch pixels show the actual dataset colour");
        }
        if (lastStyleLength != 0)
            require(poly->styleSheet().length() == lastStyleLength, "swatch updates do not accumulate stylesheet rules");
        lastStyleLength = poly->styleSheet().length();
    }
    window.close();
}
void tests(QApplication& app, const QString& output)
{
    auto vtkOutput = vtkSmartPointer<RecordingVtkOutput>::New();
    vtkOutputWindow::SetInstance(vtkOutput);
    for (const QString legacy : {QStringLiteral("System"), QStringLiteral("Light"), QStringLiteral("Dark")}) {
        QSettings settings;
        settings.clear(); // This test uses its own INI directory and organization.
        settings.setValue("appearance/theme", legacy);
        settings.setValue("view/background1", "255-255-255");
        settings.setValue("view/background2", "000-000-000");
        settings.setValue("camera/stepX", 2.5);
        settings.sync();

        auto ownedWindow = std::make_unique<OCTview3R>();
        auto& window = *ownedWindow;
        window.setAttribute(Qt::WA_DontShowOnScreen);
        window.showNormal();
        window.resize(1280, 900);
        app.processEvents();
        app.processEvents();
        auto* preferencesDock = window.findChild<QDockWidget*>("dockWidget_preferences");
        require(preferencesDock && preferencesDock->width() <= 490, "default controls dock is approximately 480 logical pixels wide");
        require(window.findChild<QComboBox*>("themeComboBox") == nullptr, "theme selector removed from Designer form");
        require(app.palette().color(QPalette::Window) == QColor(25, 33, 41), "dark theme overrides every legacy preference");
        require(app.palette().color(QPalette::Text) == QColor(225, 235, 240), "dark text contrast preserved");
        require(window.findChild<QComboBox*>("comboBox_background1")->currentText() == "255-255-255", "scene background remains independent");
        require(window.findChild<QDoubleSpinBox*>("doubleSpinBox_rotStepX")->value() == 2.5, "camera settings preserved");
        auto* scroll = window.findChild<QScrollArea*>("preferencesScrollArea");
        require(scroll && scroll->horizontalScrollBar()->maximum() == 0, "preferences fit without horizontal scrolling");
        for (const char* name : {"toolBar", "toolBar_2"}) {
            auto* bar = window.findChild<QToolBar*>(name);
            require(bar && bar->iconSize() == QSize(20, 20), "compact toolbar icons defined in Designer");
            require(bar->height() <= 34, "toolbar remains compact at the current DPI");
        }
        auto* reset = window.findChild<QPushButton*>("pushButton_reset");
        require(reset->height() <= 28 && reset->height() >= reset->fontMetrics().height() + 4, "compact button retains text height");
        auto* spin = window.findChild<QDoubleSpinBox*>("rotXCamDoubleSpinBox");
        require(spin->height() <= 26, "compact numeric control height");
        for (auto* edit : window.findChildren<QLineEdit*>()) {
            if (edit->isVisibleTo(&window))
                require(edit->height() >= edit->fontMetrics().height(), "numeric editor text is not vertically clipped");
        }
        if (legacy == "Light") {
            require(window.grab().save(output + "/main.png"), "save current main-window layout");
            require(scroll->widget()->grab().save(output + "/preferences.png"), "save full scrollable preferences");
            AboutDialog about;
            about.setAttribute(Qt::WA_DontShowOnScreen);
            about.show();
            app.processEvents();
            for (const char* name : {"authorsLabel", "acknowledgementsLabel", "licenseLabel"}) {
                auto* label = about.findChild<QLabel*>(name);
                require(label && label->height() >= label->heightForWidth(label->width()), "About text fits its wrapped label");
            }
            require(about.grab().save(output + "/about.png"), "save compact About layout");
            about.hide();
            for (const char* name : {"OpenData", "OpenPoly"}) {
                auto* dialog = window.findChild<QDialog*>(name);
                require(dialog != nullptr, "import dialog exists");
                dialog->setAttribute(Qt::WA_DontShowOnScreen);
                dialog->show();
                app.processEvents();
                require(dialog->height() >= dialog->minimumSizeHint().height(), "import dialog can fit its contents");
                require(dialog->palette().color(QPalette::Window) == QColor(25, 33, 41), "import dialog inherits dark palette");
                require(dialog->grab().save(output + "/" + name + ".png"), "save compact import dialog");
                dialog->hide();
            }
        }
        window.resize(960, 640);
        window.resizeDocks({ preferencesDock, window.findChild<QDockWidget*>("dockWidget_visualization") }, { 460, 500 }, Qt::Horizontal);
        app.processEvents();
        require(preferencesDock->width() <= 470, "controls dock can shrink to approximately 460 logical pixels");
        if (scroll->horizontalScrollBar()->maximum() != 0) {
            std::fprintf(stderr, "Small-window dimensions: viewport %dx%d, content minimum %dx%d\n",
                scroll->viewport()->width(), scroll->viewport()->height(),
                scroll->widget()->minimumSizeHint().width(), scroll->widget()->minimumSizeHint().height());
            scroll->widget()->grab().save(output + "/small-preferences.png");
        }
        require(scroll->horizontalScrollBar()->maximum() == 0, "minimum-size window avoids horizontal scrolling");
        require(scroll->viewport()->width() >= scroll->widget()->minimumSizeHint().width(), "minimum-size preferences retain their layout width");
        int shutdownRenders = 0;
        auto renderObserver = vtkSmartPointer<vtkCallbackCommand>::New();
        renderObserver->SetClientData(&shutdownRenders);
        renderObserver->SetCallback([](vtkObject*, unsigned long, void* count, void*) {
            ++*static_cast<int*>(count);
        });
        auto* vtkWidget = window.findChild<QVTKOpenGLWidget*>("qvtkWidget");
        require(vtkWidget != nullptr, "embedded VTK widget exists");
        vtkSmartPointer<vtkRenderWindow> renderWindow = vtkWidget->GetRenderWindow();
        const auto observerTag = renderWindow->AddObserver(vtkCommand::StartEvent, renderObserver);
        vtkOutput->messages.clear();
        window.close();
        app.processEvents();
        ownedWindow.reset();
        renderWindow->RemoveObserver(observerTag);
        if (!vtkOutput->messages.empty()) std::fprintf(stderr, "%s", vtkOutput->messages.c_str());
        require(vtkOutput->messages.empty(), "window teardown emits no VTK output-window diagnostics");
        require(shutdownRenders == 0, "closing and destruction do not render another frame");
        settings.sync();
        require(!settings.contains("appearance/theme"), "obsolete theme setting removed on save");
        require(settings.value("view/background1").toString() == "255-255-255", "background preference survives migration");
    }
    int savedWidth = 0;
    {
        OCTview3R window;
        window.setAttribute(Qt::WA_DontShowOnScreen);
        window.showNormal();
        window.resize(1280, 900);
        app.processEvents();
        auto* dock = window.findChild<QDockWidget*>("dockWidget_preferences");
        window.resizeDocks({ dock, window.findChild<QDockWidget*>("dockWidget_visualization") }, { 580, 700 }, Qt::Horizontal);
        app.processEvents();
        savedWidth = dock->width();
        require(savedWidth > 550, "user can widen controls after initial layout migration");
        window.close();
    }
    {
        OCTview3R window;
        window.setAttribute(Qt::WA_DontShowOnScreen);
        window.show();
        app.processEvents();
        app.processEvents();
        require(qAbs(window.findChild<QDockWidget*>("dockWidget_preferences")->width() - savedWidth) <= 2,
            "later launches preserve the user's chosen dock width");
        window.close();
    }
}
}

int main(int argc, char** argv)
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication app(argc, argv);
    const QString output = QDir::currentPath() + "/tmp/compact-ui-regression/scale-" + qEnvironmentVariable("QT_SCALE_FACTOR", "1");
    QDir().mkpath(output);
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, output);
    app.setOrganizationName("OCTview3R-ui-regression");
    app.setApplicationName("CompactUiRegression");
    app.setApplicationVersion("1.1.0");
    try {
        designerFormTests(app, output);
        tests(app, output);
        swatchStyleTests(app);
        sceneShutdownTests();
        std::printf("PASS: %d compact UI, dark-only, settings-migration and shutdown checks.\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL after %d checks: %s\n", checks, error.what());
        return 1;
    }
}
