//Unbedingt vor allen VTK-Befehlen/-Includes definieren!!!
//#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)
//#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)
#include <vtkAutoInit.h> 
VTK_MODULE_INIT(vtkRenderingVolumeOpenGL2);
VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle);
VTK_MODULE_INIT(vtkRenderingFreeType);

//https://lorensen.github.io/VTKExamples/site/Cxx/IO/ReadAllPolyDataTypes/

#include "ui_OCTview3R.h"
#include "OCTview3R.h"
#include "aboutdialog.h"
#include "Loading.h"
#include "viewerController.h"

#include <vtkImageReader2.h>
#include <vtkObject.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkImageLuminance.h>
#include <vtkMetaImageReader.h>
#include <vtkPiecewiseFunction.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
//#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkImageReader.h>
#include <vtkImageResample.h>
#include <vtkImageThreshold.h>
#include <vtkSmartPointer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageMedian3D.h>
#include <vtkTIFFReader.h>
#include <vtkMath.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkPlane.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkCubeAxesActor.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkCallbackCommand.h>
#include <vtkJPEGReader.h>
#include <vtkExtractVOI.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkAxesActor.h>
#include <vtkWindowToImageFilter.h>
#include <vtkTIFFWriter.h>
#include <vtkImplicitPlaneWidget.h>
#include <vtkClipVolume.h>
#include <vtkCutter.h>
#include <vtkImageClip.h>
//#include <vtkVolumeRayCastMapper.h>
//#include <vtkVolumeRayCastMIPFunction.h>
//#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkPlaneSource.h>
#include <vtkPlaneCollection.h>
//POINT
#include <vtkVertexGlyphFilter.h>
#include <vtkDataSetAttributes.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkActor.h>
//MESH
#include <vtkDataSetMapper.h>
#include <vtkPolygon.h>
#include <vtkCleanPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkDelaunay3D.h>
#include <vtkDelaunay2D.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataReader.h>
//DATA
#include <vtkExtractVOI.h>
#include <vtkVolumeMapper.h>
#include <vtkPolyDataMapper.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridReader.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkStructuredPointsReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkMatrix4x4.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkWarpScalar.h>
#include <vtkCubeAxesActor2D.h>
#include <vtkAxisActor2D.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkInteractorStyleTrackball.h>
#include <vtkImageReslice.h>
#include <vtkImageResliceMapper.h>
//QT
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include <QFileDialog>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QMessageBox>
#include <QPalette>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpinBox>
#include <QStyle>
#include <QStyleFactory>
#include <QTabBar>
#include <QThread>
#include <QFileInfo>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace
{
	constexpr double micrometresPerMillimetre = 1000.0;

	QString micrometreUnit()
	{
		return QString(QChar(0x00B5)) + QStringLiteral("m");
	}

	QString spacingDisplayValue(const ImageData& data, int axis)
	{
		if ((data.spacingInMillimetresMask & (1U << axis)) != 0)
		{
			return QStringLiteral("%1 %2/voxel")
				.arg(micrometresPerMillimetre * data.spacing[axis], 0, 'g', 6)
				.arg(micrometreUnit());
		}
		return QStringLiteral("%1 voxel unit")
			.arg(data.spacing[axis], 0, 'g', 6);
	}

	bool cropAxisIsCalibrated(const ImageData& data, int axis)
	{
		return data.isVolume && axis >= 0 && axis < 3 &&
			(data.spacingInMillimetresMask & (1U << axis)) != 0 &&
			data.image != nullptr && data.spacing[axis] > 0.0;
	}

	double cropDisplayValue(const ImageData& data, int axis, double voxelIndex)
	{
		if (!cropAxisIsCalibrated(data, axis))
			return voxelIndex;
		double origin[3] = {};
		data.image->GetOrigin(origin);
		return micrometresPerMillimetre *
			(origin[axis] + voxelIndex * data.spacing[axis]);
	}

	double cropVoxelIndex(const ImageData& data, int axis, double displayValue)
	{
		if (!cropAxisIsCalibrated(data, axis))
			return displayValue;
		double origin[3] = {};
		data.image->GetOrigin(origin);
		const double valueInMillimetres =
			displayValue / micrometresPerMillimetre;
		return std::round(
			(valueInMillimetres - origin[axis]) / data.spacing[axis]);
	}

	double cropStep(const ImageData& data, int axis)
	{
		if (cropAxisIsCalibrated(data, axis))
			return micrometresPerMillimetre * data.spacing[axis];
		return data.isPolyData ? 0.01 : 1.0;
	}

	double cropMinimumGap(const ImageData& data, int axis)
	{
		if (cropAxisIsCalibrated(data, axis))
			return micrometresPerMillimetre * data.spacing[axis];
		return data.isPolyData ? 0.0001 : 1.0;
	}
}

OCTview3R::OCTview3R()
	: cam(nullptr),
	  statusLabel(nullptr),
	  ui(new Ui_OCTview3R),
	  openData(nullptr),
	  openPoly(nullptr)
{
	qRegisterMetaType<vtkImageData*>("vtkImageData*");
	qRegisterMetaType<vtkPolyData*>("vtkPolyData*");

	//Initialize control-structures
	settings.oneFileLoaded			= false;
	settings.firstFileLoaded		= false;

	settings.background_RGB1[0]		= 0;
	settings.background_RGB1[1]		= 0;
	settings.background_RGB1[2]		= 0;
	settings.background_RGB2[0]		= 0;
	settings.background_RGB2[1]		= 0;
	settings.background_RGB2[2]		= 0;

	settings.showAxesBox			= false;
	settings.showAxesTriad			= false;
	settings.showOrientAxes			= false;
	settings.showScalarBar			= false;

	settings.x_rot_cam				= 0.0;
	settings.y_rot_cam				= 0.0;
	settings.z_rot_cam				= 0.0;

	onePlaneCallbackMutex			= false;

	//general
	camTrans					= vtkSmartPointer<vtkTransform>::New();
	viewerController			= std::make_unique<ViewerController>();

	//setup ui pointer
	this->ui->setupUi(this);
	setupEnhancedUi();

	//Qt GUI Initials
	Qt::WindowFlags flags = 0;
	flags = flags | Qt::WindowSystemMenuHint;
	flags = flags | Qt::CustomizeWindowHint;
	flags = flags | Qt::WindowTitleHint;
	flags = flags | Qt::WindowMinimizeButtonHint;
	flags = flags | Qt::WindowMaximizeButtonHint;
	flags = flags | Qt::WindowCloseButtonHint;
	this->setWindowFlags(flags);

	this->ui->actionObject->setCheckable(false);
	this->ui->actionPlane->setCheckable(false);
	this->ui->actionAxesTriad->setCheckable(false);
	this->ui->actionAxesBox->setCheckable(false);
	this->ui->actionScalarBar->setCheckable(false);
	this->ui->actionOrientAxes->setCheckable(false);
	this->ui->groupBox_colorMapping->setEnabled(false);
	this->ui->groupBox_threshold->setEnabled(false);
	this->ui->planeLineEdit->setReadOnly(true);
	this->ui->xLabel->setText("[0,0]");
	this->ui->yLabel->setText("[0,0]");
	this->ui->zLabel->setText("[0,0]");

	//set up action signals and slots
	connect(this->ui->actionOpenData, SIGNAL(triggered()), this, SLOT(slotOpenDataFileDialog()));
	connect(this->ui->actionOpenPolyData, SIGNAL(triggered()), this, SLOT(slotOpenPolyFileDialog()));
	connect(this->ui->actionAboutOCTview3R, SIGNAL(triggered()), this, SLOT(slotShowAbout()));
	connect(this->ui->actionExit, SIGNAL(triggered()), this, SLOT(slotExit()));
	connect(this->ui->actionObject, SIGNAL(toggled(bool)), this, SLOT(slotShowObject(bool)));
	connect(this->ui->actionPlane, SIGNAL(toggled(bool)), this, SLOT(slotShowPlane(bool)));
	connect(this->ui->actionScalarBar, SIGNAL(toggled(bool)), this, SLOT(slotShowScalarBar(bool)));
	connect(this->ui->actionAxesTriad, SIGNAL(toggled(bool)), this, SLOT(slotShowAxesTriad(bool)));
	connect(this->ui->actionAxesBox, SIGNAL(toggled(bool)), this, SLOT(slotShowAxesBox(bool)));
	connect(this->ui->actionOrientAxes, SIGNAL(toggled(bool)), this, SLOT(slotShowOrientAxes(bool)));
	connect(this->ui->actionSaveDisplay, SIGNAL(triggered()), this, SLOT(slotSaveDisplay()));
	connect(this->ui->Slider_minThreshold, SIGNAL(sliderMoved(int)), this, SLOT(slotCheckMinThresholdSlider(int)));
	connect(this->ui->Slider_maxThreshold, SIGNAL(sliderMoved(int)), this, SLOT(slotCheckMaxThresholdSlider(int)));
	connect(this->ui->Slider_minThreshold, SIGNAL(sliderReleased()), this, SLOT(slotSetThreshold()));
	connect(this->ui->Slider_maxThreshold, SIGNAL(sliderReleased()), this, SLOT(slotSetThreshold()));
	connect(this->ui->Slider_objectOpacity, SIGNAL(valueChanged(int)), this, SLOT(slotSetObjectOpacity(int)));
	connect(this->ui->Slider_windowWidth, SIGNAL(valueChanged(int)), this, SLOT(slotSetWindowWidth(int)));
	connect(this->ui->Slider_windowLevel, SIGNAL(valueChanged(int)), this, SLOT(slotSetWindowLevel(int)));
	connect(this->ui->Slider_polyGloss, SIGNAL(valueChanged(int)), this, SLOT(slotSetPolyGloss(int)));
	connect(this->ui->comboBox_colormapStyle, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSetColormap(QString)));
	connect(this->ui->pushButton_pickPolyColor, SIGNAL(clicked()), this, SLOT(slotPickPolyColor()));
	connect(this->ui->pushButton_pickVolumeColor, SIGNAL(clicked()), this, SLOT(slotPickVolumeColor()));
	connect(this->ui->checkBox_adjustColormap, SIGNAL(clicked(bool)), this, SLOT(slotAdjustColormap(bool)));
	connect(this->ui->checkBox_invertColormap, SIGNAL(clicked(bool)), this, SLOT(slotInvertColormap(bool)));
	connect(this->ui->spinBox_pointSize, SIGNAL(valueChanged(int)), this, SLOT(slotSetPointSize(int)));
	connect(this->ui->groupBox_object, SIGNAL(clicked(bool)), this, SLOT(slotShowObject(bool)));
	connect(this->ui->groupBox_plane, SIGNAL(clicked(bool)), this, SLOT(slotShowPlane(bool)));
	connect(this->ui->planeMedianCheckBox, SIGNAL(clicked(bool)), this, SLOT(slotMedianCheckBox(bool)));
	connect(this->ui->kernelXSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotKernelXChanged(int)));
	connect(this->ui->kernelYSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotKernelYChanged(int)));
	connect(this->ui->kernelZSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotKernelZChanged(int)));
	connect(this->ui->planeOrientationComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOrientationChanged(int)));
	connect(this->ui->planePushButton, SIGNAL(clicked()), this, SLOT(slotGetPlaneData()));
	connect(this->ui->pushButton_render, SIGNAL(clicked()), this, SLOT(slotApplyRanges()));
	connect(this->ui->actionX, SIGNAL(triggered()), this, SLOT(slotFrontX())); 
	connect(this->ui->actionY, SIGNAL(triggered()), this, SLOT(slotFrontY())); 
	connect(this->ui->actionZ, SIGNAL(triggered()), this, SLOT(slotFrontZ())); 
	connect(this->ui->actionXu, SIGNAL(triggered()), this, SLOT(slotBackX())); 
	connect(this->ui->actionYu, SIGNAL(triggered()), this, SLOT(slotBackY())); 
	connect(this->ui->actionZu, SIGNAL(triggered()), this, SLOT(slotBackZ())); 
	connect(this->ui->actionRot, SIGNAL(triggered()), this, SLOT(slotRot90())); 
	connect(this->ui->upPushButton, SIGNAL(clicked()), this, SLOT(slotPlaneUp())); 
	connect(this->ui->downPushButton, SIGNAL(clicked()), this, SLOT(slotPlaneDown())); 
	connect(this->ui->pushButton_save, SIGNAL(clicked()), this, SLOT(slotSaveDisplay()));
	connect(this->ui->comboBox_blendMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetBlendMode(int)));
	connect(this->ui->comboBox_colorMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetColorMode(int)));
	connect(this->ui->comboBox_polyMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetPolyMode(int)));
	connect(this->ui->checkBox_flipPlane, SIGNAL(clicked(bool)), this, SLOT(slotCheckFlipPlane(bool)));
	connect(this->ui->scaleXDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotScaleX(double)));
	connect(this->ui->scaleYDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotScaleY(double)));
	connect(this->ui->scaleZDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotScaleZ(double)));
	connect(this->ui->rotXCamDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotRotCamX(double)));
	connect(this->ui->rotYCamDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotRotCamY(double)));
	connect(this->ui->rotZCamDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotRotCamZ(double)));
	connect(this->ui->doubleSpinBox_rotStepX, SIGNAL(valueChanged(double)), this, SLOT(slotRotCamStepX(double)));
	connect(this->ui->doubleSpinBox_rotStepY, SIGNAL(valueChanged(double)), this, SLOT(slotRotCamStepY(double)));
	connect(this->ui->doubleSpinBox_rotStepZ, SIGNAL(valueChanged(double)), this, SLOT(slotRotCamStepZ(double)));
	connect(this->ui->rotXDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotRotX(double)));
	connect(this->ui->rotYDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotRotY(double)));
	connect(this->ui->rotZDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotRotZ(double)));
	connect(this->ui->shiftXDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotShiftX(double)));
	connect(this->ui->shiftYDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotShiftY(double)));
	connect(this->ui->shiftZDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotShiftZ(double)));
	connect(this->ui->pushButton_reset, SIGNAL(clicked()), this, SLOT(slotResetCam()));
	connect(this->ui->planVisibilityCheckBox, SIGNAL(clicked(bool)), this, SLOT(slotPlaneVisibility(bool)));
	connect(this->ui->comboBox_background1, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotBackground1(QString)));
	connect(this->ui->comboBox_background2, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotBackground2(QString)));
	connect(this->ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotSetImageData(int)));

	openData = new OpenData(this);
	connect(openData, SIGNAL(signalStartProcess()), this, SLOT(slotProcessDataFile()));
	openPoly = new OpenPoly(this);
	connect(openPoly, SIGNAL(signalStartProcess()), this, SLOT(slotProcessPolyFile()));

	//setup statusBar
	statusLabel = new QLabel();
	this->ui->statusBar->addWidget(statusLabel);
	statusLabel->setText("No file loaded.");

	initializeVTKPipeline();
	loadApplicationSettings();
}

void OCTview3R::setupEnhancedUi()
{
	metadataLabel = ui->metadataLabel;
	themeComboBox = ui->themeComboBox;
	opacitySpinBox = ui->opacitySpinBox;
	glossSpinBox = ui->glossSpinBox;
	minThresholdSpinBox = ui->minThresholdSpinBox;
	maxThresholdSpinBox = ui->maxThresholdSpinBox;
	resetRangesButton = ui->resetRangesButton;
	resetTransformButton = ui->resetTransformButton;
	rangeGroupBox = ui->rangeGroupBox;
	fitSelectedAction = ui->actionFitSelected;
	fitAllAction = ui->actionFitAll;
	parallelProjectionAction = ui->actionParallelProjection;

	connect(
		ui->tabWidget,
		&QTabWidget::tabCloseRequested,
		this,
		static_cast<void (OCTview3R::*)(int)>(&OCTview3R::slotCloseTab));

	setupGeneralPanel();
	setupObjectPanel();
	setupNumericEditors();
	setupCameraToolbar();
	setupMetadataPanel();
}

void OCTview3R::setupGeneralPanel()
{
	connect(
		ui->themeComboBox,
		&QComboBox::currentTextChanged,
		this,
		&OCTview3R::slotThemeChanged);
}

void OCTview3R::setupObjectPanel()
{
	QDoubleSpinBox* rangeMin[] = {
		ui->x0DoubleSpinBox,
		ui->y0DoubleSpinBox,
		ui->z0DoubleSpinBox
	};
	QDoubleSpinBox* rangeMax[] = {
		ui->x1DoubleSpinBox,
		ui->y1DoubleSpinBox,
		ui->z1DoubleSpinBox
	};
	connect(
		ui->resetTransformButton,
		&QPushButton::clicked,
		this,
		&OCTview3R::slotResetObjectTransform);
	connect(
		ui->resetRangesButton,
		&QPushButton::clicked,
		this,
		&OCTview3R::slotResetRanges);
	for (QDoubleSpinBox* spinBox : rangeMin)
		connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &OCTview3R::slotRangesEdited);
	for (QDoubleSpinBox* spinBox : rangeMax)
		connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &OCTview3R::slotRangesEdited);
}

void OCTview3R::setupNumericEditors()
{
	connect(
		ui->Slider_objectOpacity,
		&QSlider::valueChanged,
		ui->opacitySpinBox,
		&QSpinBox::setValue);
	connect(
		ui->opacitySpinBox,
		QOverload<int>::of(&QSpinBox::valueChanged),
		ui->Slider_objectOpacity,
		&QSlider::setValue);
	connect(
		ui->Slider_windowWidth,
		&QSlider::valueChanged,
		ui->windowWidthSpinBox,
		&QSpinBox::setValue);
	connect(
		ui->windowWidthSpinBox,
		QOverload<int>::of(&QSpinBox::valueChanged),
		ui->Slider_windowWidth,
		&QSlider::setValue);
	connect(
		ui->Slider_windowLevel,
		&QSlider::valueChanged,
		ui->windowLevelSpinBox,
		&QSpinBox::setValue);
	connect(
		ui->windowLevelSpinBox,
		QOverload<int>::of(&QSpinBox::valueChanged),
		ui->Slider_windowLevel,
		&QSlider::setValue);
	connect(
		ui->Slider_polyGloss,
		&QSlider::valueChanged,
		ui->glossSpinBox,
		&QSpinBox::setValue);
	connect(
		ui->glossSpinBox,
		QOverload<int>::of(&QSpinBox::valueChanged),
		ui->Slider_polyGloss,
		&QSlider::setValue);

	connect(
		ui->Slider_minThreshold,
		&QSlider::valueChanged,
		ui->minThresholdSpinBox,
		&QSpinBox::setValue);
	connect(
		ui->minThresholdSpinBox,
		QOverload<int>::of(&QSpinBox::valueChanged),
		ui->Slider_minThreshold,
		&QSlider::setValue);
	connect(
		ui->Slider_maxThreshold,
		&QSlider::valueChanged,
		ui->maxThresholdSpinBox,
		&QSpinBox::setValue);
	connect(
		ui->maxThresholdSpinBox,
		QOverload<int>::of(&QSpinBox::valueChanged),
		ui->Slider_maxThreshold,
		&QSlider::setValue);
	connect(
		ui->minThresholdSpinBox,
		&QSpinBox::editingFinished,
		this,
		&OCTview3R::slotSetThreshold);
	connect(
		ui->maxThresholdSpinBox,
		&QSpinBox::editingFinished,
		this,
		&OCTview3R::slotSetThreshold);
}

void OCTview3R::setupCameraToolbar()
{
	connect(
		ui->actionFitSelected,
		&QAction::triggered,
		this,
		&OCTview3R::slotFitSelected);
	connect(
		ui->actionFitAll,
		&QAction::triggered,
		this,
		&OCTview3R::slotFitAll);
	connect(
		ui->actionParallelProjection,
		&QAction::toggled,
		this,
		&OCTview3R::slotParallelProjection);
}

void OCTview3R::setupMetadataPanel()
{
	// Static layout and properties are defined in OCTview3R.ui.
}

OCTview3R::~OCTview3R()
{
	for (QThread* thread : loadingThreads)
	{
		thread->requestInterruption();
		thread->quit();
	}
	for (QThread* thread : loadingThreads)
		thread->wait();
	loadingThreads.clear();

	if (viewerController)
		viewerController->clear(documentModel);
	documentModel.clear();

	delete ui;
}

void OCTview3R::closeEvent(QCloseEvent* event)
{
	saveApplicationSettings();
	QMainWindow::closeEvent(event);
}

void OCTview3R::loadApplicationSettings()
{
	QSettings applicationSettings;
	const QByteArray geometry =
		applicationSettings.value(QStringLiteral("window/geometry")).toByteArray();
	if (geometry.isEmpty())
		showMaximized();
	else
		restoreGeometry(geometry);
	const QByteArray state =
		applicationSettings.value(QStringLiteral("window/state")).toByteArray();
	if (!state.isEmpty())
		restoreState(state);

	currentTheme = applicationSettings
		.value(QStringLiteral("appearance/theme"), QStringLiteral("System"))
		.toString();
	if (themeComboBox->findText(currentTheme) < 0)
		currentTheme = QStringLiteral("System");
	{
		const QSignalBlocker blocker(themeComboBox);
		themeComboBox->setCurrentText(currentTheme);
	}
	applyTheme(currentTheme);

	const QString background1 = applicationSettings
		.value(QStringLiteral("view/background1"), QStringLiteral("000-000-000"))
		.toString();
	const QString background2 = applicationSettings
		.value(QStringLiteral("view/background2"), QStringLiteral("000-000-000"))
		.toString();
	{
		const QSignalBlocker blockFirst(ui->comboBox_background1);
		const QSignalBlocker blockSecond(ui->comboBox_background2);
		ui->comboBox_background1->setCurrentText(background1);
		ui->comboBox_background2->setCurrentText(background2);
	}
	slotBackground1(background1);
	slotBackground2(background2);

	ui->doubleSpinBox_rotStepX->setValue(
		applicationSettings.value(QStringLiteral("camera/stepX"), 1.0).toDouble());
	ui->doubleSpinBox_rotStepY->setValue(
		applicationSettings.value(QStringLiteral("camera/stepY"), 1.0).toDouble());
	ui->doubleSpinBox_rotStepZ->setValue(
		applicationSettings.value(QStringLiteral("camera/stepZ"), 1.0).toDouble());
	const bool parallel = applicationSettings
		.value(QStringLiteral("camera/parallelProjection"), false)
		.toBool();
	parallelProjectionAction->setChecked(parallel);

	settings.showAxesTriad = applicationSettings
		.value(QStringLiteral("view/showAxes"), false)
		.toBool();
	settings.showAxesBox = applicationSettings
		.value(QStringLiteral("view/showBox"), false)
		.toBool();
	settings.showOrientAxes = applicationSettings
		.value(QStringLiteral("view/showOrientation"), false)
		.toBool();
	settings.showScalarBar = applicationSettings
		.value(QStringLiteral("view/showScalarBar"), false)
		.toBool();

	openData->setFilePath(applicationSettings
		.value(QStringLiteral("paths/volume"), QStringLiteral("."))
		.toString());
	openPoly->setFilePath(applicationSettings
		.value(QStringLiteral("paths/poly"), QStringLiteral("."))
		.toString());
}

void OCTview3R::saveApplicationSettings() const
{
	QSettings applicationSettings;
	applicationSettings.setValue(QStringLiteral("window/geometry"), saveGeometry());
	applicationSettings.setValue(QStringLiteral("window/state"), saveState());
	applicationSettings.setValue(QStringLiteral("appearance/theme"), currentTheme);
	applicationSettings.setValue(
		QStringLiteral("view/background1"),
		ui->comboBox_background1->currentText());
	applicationSettings.setValue(
		QStringLiteral("view/background2"),
		ui->comboBox_background2->currentText());
	applicationSettings.setValue(
		QStringLiteral("camera/stepX"),
		ui->doubleSpinBox_rotStepX->value());
	applicationSettings.setValue(
		QStringLiteral("camera/stepY"),
		ui->doubleSpinBox_rotStepY->value());
	applicationSettings.setValue(
		QStringLiteral("camera/stepZ"),
		ui->doubleSpinBox_rotStepZ->value());
	applicationSettings.setValue(
		QStringLiteral("camera/parallelProjection"),
		parallelProjectionAction->isChecked());
	applicationSettings.setValue(
		QStringLiteral("view/showAxes"),
		settings.showAxesTriad);
	applicationSettings.setValue(
		QStringLiteral("view/showBox"),
		settings.showAxesBox);
	applicationSettings.setValue(
		QStringLiteral("view/showOrientation"),
		settings.showOrientAxes);
	applicationSettings.setValue(
		QStringLiteral("view/showScalarBar"),
		settings.showScalarBar);
	applicationSettings.setValue(
		QStringLiteral("paths/volume"),
		openData->getFilePath());
	applicationSettings.setValue(
		QStringLiteral("paths/poly"),
		openPoly->getFilePath());
}

void OCTview3R::applyTheme(const QString& theme)
{
	static const QPalette systemPalette = qApp->palette();
	static const QString systemStyleSheet = qApp->styleSheet();
	static const QString systemStyleName = qApp->style()->objectName();

	const bool useDarkTheme = theme == QStringLiteral("Dark");
	const QString requestedStyle =
		useDarkTheme ? QStringLiteral("Fusion") : systemStyleName;
	if (QStyle* style = QStyleFactory::create(requestedStyle))
		qApp->setStyle(style);

	QPalette palette = systemPalette;
	QString styleSheet = systemStyleSheet;
	if (useDarkTheme)
	{
		// Palette values and parts of the stylesheet are adapted from
		// Qt-Frameless-Window-DarkStyle (MIT), as used by UKE-smartLab.
		palette.setColor(QPalette::Window, QColor(53, 53, 53));
		palette.setColor(QPalette::WindowText, Qt::white);
		palette.setColor(
			QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
		palette.setColor(QPalette::Base, QColor(42, 42, 42));
		palette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
		palette.setColor(QPalette::ToolTipBase, Qt::white);
		palette.setColor(QPalette::ToolTipText, QColor(53, 53, 53));
		palette.setColor(QPalette::Text, Qt::white);
		palette.setColor(
			QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
		palette.setColor(QPalette::Dark, QColor(35, 35, 35));
		palette.setColor(QPalette::Shadow, QColor(20, 20, 20));
		palette.setColor(QPalette::Mid, QColor(80, 80, 80));
		palette.setColor(QPalette::Midlight, QColor(95, 95, 95));
		palette.setColor(QPalette::Button, QColor(53, 53, 53));
		palette.setColor(QPalette::ButtonText, Qt::white);
		palette.setColor(
			QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
		palette.setColor(QPalette::BrightText, Qt::red);
		palette.setColor(QPalette::Link, QColor(42, 130, 218));
		palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
		palette.setColor(
			QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
		palette.setColor(QPalette::HighlightedText, Qt::white);
		palette.setColor(
			QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));
		palette.setColor(QPalette::PlaceholderText, QColor(155, 155, 155));

		QFile styleFile(
			QStringLiteral(":/OCTview3R/Resources/darkstyle/darkstyle.qss"));
		if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			if (!styleSheet.isEmpty())
				styleSheet += QLatin1Char('\n');
			styleSheet += QString::fromUtf8(styleFile.readAll());
		}
	}
	else if (theme == QStringLiteral("Light"))
	{
		palette.setColor(QPalette::Window, QColor(245, 246, 248));
		palette.setColor(QPalette::WindowText, QColor(28, 30, 33));
		palette.setColor(QPalette::Base, Qt::white);
		palette.setColor(QPalette::AlternateBase, QColor(236, 238, 241));
		palette.setColor(QPalette::Text, QColor(28, 30, 33));
		palette.setColor(QPalette::Button, QColor(239, 241, 244));
		palette.setColor(QPalette::ButtonText, QColor(28, 30, 33));
		palette.setColor(QPalette::Highlight, QColor(33, 118, 174));
		palette.setColor(QPalette::HighlightedText, Qt::white);
	}
	qApp->setPalette(palette);
	qApp->setStyleSheet(styleSheet);
	currentTheme = theme;
	if (viewerController)
		viewerController->updateAnnotationColor();
}

void OCTview3R::slotThemeChanged(const QString& theme)
{
	applyTheme(theme);
}

void OCTview3R::markTransformDirty()
{
	if (activeImageData)
		activeImageData->transformDirty = true;
}

void OCTview3R::markAppearanceDirty()
{
	if (activeImageData)
		activeImageData->appearanceDirty = true;
}

void OCTview3R::markDataPipelineDirty()
{
	if (activeImageData)
		activeImageData->dataPipelineDirty = true;
}

void OCTview3R::markPlaneDirty()
{
	if (activeImageData)
		activeImageData->planeDirty = true;
}

void OCTview3R::setRangesPending(bool pending)
{
	ui->pushButton_render->setProperty("pending", pending);
	ui->pushButton_render->style()->unpolish(ui->pushButton_render);
	ui->pushButton_render->style()->polish(ui->pushButton_render);
	ui->pushButton_render->update();
}

void OCTview3R::applyAutomaticWindowLevel()
{
	if (activeImageData == nullptr || !activeImageData->isVolume)
		return;

	activeImageData->windowWidth = std::max(
		1,
		activeImageData->currentMaxThreshold -
			activeImageData->currentMinThreshold + 1);
	activeImageData->windowLevel = static_cast<int>(std::round(
		0.5 * (static_cast<double>(activeImageData->currentMinThreshold) +
			activeImageData->currentMaxThreshold + 1.0)));

	const QSignalBlocker blockWindowSlider(ui->Slider_windowWidth);
	const QSignalBlocker blockWindowSpinBox(ui->windowWidthSpinBox);
	const QSignalBlocker blockLevelSlider(ui->Slider_windowLevel);
	const QSignalBlocker blockLevelSpinBox(ui->windowLevelSpinBox);
	ui->Slider_windowWidth->setValue(activeImageData->windowWidth);
	ui->windowWidthSpinBox->setValue(activeImageData->windowWidth);
	ui->Slider_windowLevel->setValue(activeImageData->windowLevel);
	ui->windowLevelSpinBox->setValue(activeImageData->windowLevel);
}

void OCTview3R::updateRangePresentation()
{
	if (!activeImageData)
	{
		rangeGroupBox->setTitle(tr("Crop ranges"));
		return;
	}
	if (activeImageData->isPolyData)
		rangeGroupBox->setTitle(tr("Crop ranges (data coordinates)"));
	else if (activeImageData->spacingInMillimetresMask == 0x7U)
		rangeGroupBox->setTitle(
			tr("Crop ranges (%1)").arg(micrometreUnit()));
	else if (activeImageData->spacingInMillimetresMask != 0)
		rangeGroupBox->setTitle(
			tr("Crop ranges (%1 where calibrated)").arg(micrometreUnit()));
	else
		rangeGroupBox->setTitle(tr("Crop ranges (voxel indices)"));
}

void OCTview3R::updateColorModeControls()
{
	const bool isVolume = activeImageData != nullptr &&
		activeImageData->fileLoaded && activeImageData->isVolume;
	const bool rgbAvailable = isVolume && activeImageData->image != nullptr &&
		activeImageData->image->GetNumberOfScalarComponents() >= 3;
	const bool showRgb = rgbAvailable && activeImageData->renderRgb;

	ui->colorModeLabel->setEnabled(rgbAvailable);
	ui->comboBox_colorMode->setEnabled(rgbAvailable);
	ui->comboBox_colorMode->setCurrentIndex(showRgb ? 1 : 0);
	ui->colormapOptionsLabel->setEnabled(isVolume && !showRgb);
	ui->comboBox_colormapStyle->setEnabled(isVolume && !showRgb);
	ui->checkBox_invertColormap->setEnabled(isVolume && !showRgb);
	ui->pushButton_pickVolumeColor->setEnabled(isVolume && !showRgb);
}

void OCTview3R::updateMetadata()
{
	if (!metadataLabel || !activeImageData || !activeImageData->fileLoaded)
	{
		if (metadataLabel)
			metadataLabel->setText(tr("No dataset selected."));
		return;
	}

	const QFileInfo fileInfo(activeImageData->fileName);
	const double sizeMiB =
		static_cast<double>(fileInfo.size()) / (1024.0 * 1024.0);
	QString text = tr("File: %1 (%2 MiB)\n")
		.arg(fileInfo.fileName())
		.arg(sizeMiB, 0, 'f', 2);
	if (activeImageData->isVolume)
	{
		double scalarRange[2] = {};
		activeImageData->image->GetScalarRange(scalarRange);
		text += tr("Type: Volume (%1)\n").arg(activeImageData->typeName);
		text += tr("Dimensions: %1 x %2 x %3\n")
			.arg(activeImageData->width)
			.arg(activeImageData->height)
			.arg(activeImageData->depth);
		text += tr("Spacing: X %1, Y %2, Z %3\n")
			.arg(spacingDisplayValue(*activeImageData, 0))
			.arg(spacingDisplayValue(*activeImageData, 1))
			.arg(spacingDisplayValue(*activeImageData, 2));
		text += tr("Scalars: %1, %2 component(s), range [%3, %4]")
			.arg(QString::fromLatin1(activeImageData->image->GetScalarTypeAsString()))
			.arg(activeImageData->image->GetNumberOfScalarComponents())
			.arg(scalarRange[0], 0, 'g', 8)
			.arg(scalarRange[1], 0, 'g', 8);
		text += tr("\nColor mode: %1")
			.arg(activeImageData->renderRgb ? tr("RGB") : tr("Grayscale"));
	}
	else
	{
		text += tr("Type: PolyData\n");
		text += tr("Points: %1   Cells: %2\n")
			.arg(activeImageData->poly->GetNumberOfPoints())
			.arg(activeImageData->poly->GetNumberOfCells());
		text += tr("Polygons: %1   Lines: %2\n")
			.arg(activeImageData->poly->GetNumberOfPolys())
			.arg(activeImageData->poly->GetNumberOfLines());
		text += tr("Bounds: X [%1, %2], Y [%3, %4], Z [%5, %6]")
			.arg(activeImageData->sourceVOI[0], 0, 'g', 6)
			.arg(activeImageData->sourceVOI[1], 0, 'g', 6)
			.arg(activeImageData->sourceVOI[2], 0, 'g', 6)
			.arg(activeImageData->sourceVOI[3], 0, 'g', 6)
			.arg(activeImageData->sourceVOI[4], 0, 'g', 6)
			.arg(activeImageData->sourceVOI[5], 0, 'g', 6);
	}
	metadataLabel->setText(text);
	metadataLabel->setToolTip(activeImageData->fileName);
}

void OCTview3R::addDocumentTab(ImageData& data)
{
	if (documentModel.size() == 1 && ui->tabWidget->count() == 1)
	{
		QWidget* placeholder = ui->tabWidget->widget(0);
		ui->tabWidget->removeTab(0);
		if (placeholder)
			placeholder->deleteLater();
	}
	auto* page = new QWidget(ui->tabWidget);
	const QString baseName = QFileInfo(data.fileName).fileName();
	const int index = ui->tabWidget->addTab(page, baseName);
	ui->tabWidget->setTabToolTip(index, data.fileName);
	ui->tabWidget->setTabsClosable(true);
	ui->tabWidget->setCurrentIndex(index);
}

void OCTview3R::slotResetCam()
{
	if (!viewerController || !viewerController->renderer())
		return;

	viewerController->renderer()->ResetCamera();
	settings.x_rot_cam = 0.0;
	settings.y_rot_cam = 0.0;
	settings.z_rot_cam = 0.0;
	const QSignalBlocker blockRotX(ui->rotXCamDoubleSpinBox);
	const QSignalBlocker blockRotY(ui->rotYCamDoubleSpinBox);
	const QSignalBlocker blockRotZ(ui->rotZCamDoubleSpinBox);
	ui->rotXCamDoubleSpinBox->setValue(0.0);
	ui->rotYCamDoubleSpinBox->setValue(0.0);
	ui->rotZCamDoubleSpinBox->setValue(0.0);
	viewerController->render();
}
void OCTview3R::initializeVTKPipeline()
{
	viewerController->initialize(this->ui->qvtkWidget->GetRenderWindow(), settings);
	slotResetCam();
}
void OCTview3R::slotSetImageData(int index)
{
	if (index < 0 || index >= documentModel.size())
		return;

	documentModel.setActiveIndex(index);
	slotSetImageData(documentModel.at(index));
	viewerController->refreshDecorations(documentModel, activeImageData, settings);
}

void OCTview3R::slotSetImageData(ImageData* data)
{
	if (data == nullptr)
		return;

	// Updating one document's controls must not invoke handlers for every
	// programmatically assigned value.
	std::vector<std::unique_ptr<QSignalBlocker>> signalBlockers;
	const QList<QObject*> objects = findChildren<QObject*>();
	signalBlockers.reserve(static_cast<std::size_t>(objects.size()));
	for (QObject* object : objects)
		signalBlockers.emplace_back(std::make_unique<QSignalBlocker>(object));

	if(data->fileLoaded){
		//set activeImageData to new data
		activeImageData = data;
		documentModel.setActiveIndex(documentModel.indexOf(data));

		//allow actions
		this->ui->actionScalarBar->setCheckable(true);
		this->ui->actionAxesBox->setCheckable(true);
		this->ui->actionAxesTriad->setCheckable(true);
		this->ui->actionOrientAxes->setCheckable(true);
		this->ui->actionScalarBar->setChecked(settings.showScalarBar);
		this->ui->actionAxesBox->setChecked(settings.showAxesBox);
		this->ui->actionAxesTriad->setChecked(settings.showAxesTriad);
		this->ui->actionOrientAxes->setChecked(settings.showOrientAxes);

		//object
		this->ui->actionObject->setCheckable(true);
		this->ui->actionObject->setChecked(activeImageData->showObject);
		this->ui->groupBox_object->setChecked(activeImageData->showObject);
		if(activeImageData->isVolume){
			ui->widget_poly->setEnabled(false);
			ui->widget_volume->setEnabled(true);
		}else if(activeImageData->isPolyData){
			ui->widget_poly->setEnabled(true);
			ui->widget_volume->setEnabled(false);
		}else{
			ui->widget_poly->setEnabled(false);
			ui->widget_volume->setEnabled(false);
		}

		//Set range elements
		this->ui->groupBox_object->setEnabled(true);
		const bool rangesSupported =
			activeImageData->isVolume || activeImageData->isPolyData;
		QDoubleSpinBox* lowerRangeControls[3] = {
			ui->x0DoubleSpinBox, ui->y0DoubleSpinBox, ui->z0DoubleSpinBox
		};
		QDoubleSpinBox* upperRangeControls[3] = {
			ui->x1DoubleSpinBox, ui->y1DoubleSpinBox, ui->z1DoubleSpinBox
		};
		QLabel* rangeLabels[3] = { ui->xLabel, ui->yLabel, ui->zLabel };
		for (int axis = 0; axis < 3; ++axis)
		{
			const int lowerIndex = 2 * axis;
			const int upperIndex = lowerIndex + 1;
			const int decimals = cropAxisIsCalibrated(*activeImageData, axis)
				? 2
				: (activeImageData->isPolyData ? 4 : 0);
			const QString suffix = cropAxisIsCalibrated(*activeImageData, axis)
				? QStringLiteral(" ") + micrometreUnit()
				: QString();
			const double step = cropStep(*activeImageData, axis);
			const double gap = cropMinimumGap(*activeImageData, axis);
			const double sourceLower = cropDisplayValue(
				*activeImageData, axis, activeImageData->sourceVOI[lowerIndex]);
			const double sourceUpper = cropDisplayValue(
				*activeImageData, axis, activeImageData->sourceVOI[upperIndex]);
			const double currentLower = cropDisplayValue(
				*activeImageData, axis, activeImageData->VOI[lowerIndex]);
			const double currentUpper = cropDisplayValue(
				*activeImageData, axis, activeImageData->VOI[upperIndex]);

			lowerRangeControls[axis]->setDecimals(decimals);
			upperRangeControls[axis]->setDecimals(decimals);
			lowerRangeControls[axis]->setSuffix(suffix);
			upperRangeControls[axis]->setSuffix(suffix);
			lowerRangeControls[axis]->setSingleStep(step);
			upperRangeControls[axis]->setSingleStep(step);
			lowerRangeControls[axis]->setEnabled(rangesSupported);
			upperRangeControls[axis]->setEnabled(rangesSupported);
			lowerRangeControls[axis]->setRange(sourceLower, sourceUpper - gap);
			upperRangeControls[axis]->setRange(sourceLower + gap, sourceUpper);
			lowerRangeControls[axis]->setValue(currentLower);
			upperRangeControls[axis]->setValue(currentUpper);
			rangeLabels[axis]->setText(
				"[" + QString::number(sourceLower, 'f', 2) +
				"," + QString::number(sourceUpper, 'f', 2) + "]");
		}
		this->ui->pushButton_render->setEnabled(rangesSupported);
		{
			const QSignalBlocker blockRotX(this->ui->rotXDoubleSpinBox);
			const QSignalBlocker blockRotY(this->ui->rotYDoubleSpinBox);
			const QSignalBlocker blockRotZ(this->ui->rotZDoubleSpinBox);
			const QSignalBlocker blockShiftX(this->ui->shiftXDoubleSpinBox);
			const QSignalBlocker blockShiftY(this->ui->shiftYDoubleSpinBox);
			const QSignalBlocker blockShiftZ(this->ui->shiftZDoubleSpinBox);
			this->ui->rotXDoubleSpinBox->setValue(activeImageData->rot[0]);
			this->ui->rotYDoubleSpinBox->setValue(activeImageData->rot[1]);
			this->ui->rotZDoubleSpinBox->setValue(activeImageData->rot[2]);
			this->ui->shiftXDoubleSpinBox->setValue(activeImageData->shift[0]);
			this->ui->shiftYDoubleSpinBox->setValue(activeImageData->shift[1]);
			this->ui->shiftZDoubleSpinBox->setValue(activeImageData->shift[2]);
			this->ui->scaleXDoubleSpinBox->setValue(activeImageData->scale[0]);
			this->ui->scaleYDoubleSpinBox->setValue(activeImageData->scale[1]);
			this->ui->scaleZDoubleSpinBox->setValue(activeImageData->scale[2]);
		}
		//Set colormap values
		this->ui->groupBox_colorMapping->setEnabled(true);
		this->ui->comboBox_blendMode->setCurrentIndex(activeImageData->blendMode);
		this->ui->comboBox_colormapStyle->setCurrentText(activeImageData->colormapName);
		this->ui->comboBox_polyMode->setCurrentIndex(activeImageData->polyMode);
		this->ui->spinBox_pointSize->setMinimum(1);
		this->ui->spinBox_pointSize->setValue(activeImageData->pointSize);
		this->ui->pushButton_pickPolyColor->setStyleSheet("background-color: "+activeImageData->polyColor.name());
		this->ui->pushButton_pickVolumeColor->setStyleSheet("background-color: "+activeImageData->volumeColor.name());
		this->ui->Slider_objectOpacity->setValue(int(100*activeImageData->objectOpacity));
		this->opacitySpinBox->setValue(this->ui->Slider_objectOpacity->value());
		const bool windowLevelSupported = activeImageData->isVolume;
		const int maximumWindow = std::max(
			1,
			activeImageData->maxValue - activeImageData->minValue + 1);
		this->ui->label_windowWidth->setEnabled(windowLevelSupported);
		this->ui->Slider_windowWidth->setEnabled(windowLevelSupported);
		this->ui->windowWidthSpinBox->setEnabled(windowLevelSupported);
		this->ui->label_windowLevel->setEnabled(windowLevelSupported);
		this->ui->Slider_windowLevel->setEnabled(windowLevelSupported);
		this->ui->windowLevelSpinBox->setEnabled(windowLevelSupported);
		this->ui->Slider_windowWidth->setRange(1, maximumWindow);
		this->ui->windowWidthSpinBox->setRange(1, maximumWindow);
		this->ui->Slider_windowLevel->setRange(
			activeImageData->minValue,
			activeImageData->maxValue);
		this->ui->windowLevelSpinBox->setRange(
			activeImageData->minValue,
			activeImageData->maxValue);
		this->ui->Slider_windowWidth->setValue(activeImageData->windowWidth);
		this->ui->windowWidthSpinBox->setValue(activeImageData->windowWidth);
		this->ui->Slider_windowLevel->setValue(activeImageData->windowLevel);
		this->ui->windowLevelSpinBox->setValue(activeImageData->windowLevel);
		this->ui->Slider_polyGloss->setEnabled(activeImageData->isPolyData);
		this->ui->label_polyGloss->setEnabled(activeImageData->isPolyData);
		this->glossSpinBox->setEnabled(activeImageData->isPolyData);
		this->ui->Slider_polyGloss->setValue(
			static_cast<int>(std::round(100.0 * activeImageData->polyGloss)));
		this->glossSpinBox->setValue(this->ui->Slider_polyGloss->value());
		this->ui->checkBox_adjustColormap->setChecked(activeImageData->adjustColormap);
		this->ui->checkBox_invertColormap->setChecked(activeImageData->invertColormap);
		updateColorModeControls();

		//Set threshold values
		this->ui->groupBox_threshold->setEnabled(activeImageData->isVolume);
		this->ui->Slider_minThreshold->setMinimum(activeImageData->minValue);
		this->ui->Slider_minThreshold->setMaximum(activeImageData->maxValue);
		this->ui->Slider_minThreshold->setValue(activeImageData->currentMinThreshold);
		this->ui->Slider_minThreshold->setEnabled(activeImageData->isVolume);
		this->minThresholdSpinBox->setRange(
			activeImageData->minValue,
			activeImageData->maxValue);
		this->minThresholdSpinBox->setValue(activeImageData->currentMinThreshold);
		this->ui->Slider_maxThreshold->setMinimum(activeImageData->minValue);
		this->ui->Slider_maxThreshold->setMaximum(activeImageData->maxValue);
		this->ui->Slider_maxThreshold->setValue(activeImageData->currentMaxThreshold);
		this->ui->Slider_maxThreshold->setEnabled(activeImageData->isVolume);
		this->maxThresholdSpinBox->setRange(
			activeImageData->minValue,
			activeImageData->maxValue);
		this->maxThresholdSpinBox->setValue(activeImageData->currentMaxThreshold);
		this->minThresholdSpinBox->setEnabled(activeImageData->isVolume);
		this->maxThresholdSpinBox->setEnabled(activeImageData->isVolume);

		//plane orientation and median kernel
		const bool planeSupported = activeImageData->isVolume;
		if (!planeSupported)
			activeImageData->showPlane = false;
		this->ui->groupBox_plane->setEnabled(planeSupported);
		this->ui->actionPlane->setCheckable(planeSupported);
		this->ui->actionPlane->setChecked(activeImageData->showPlane);
		this->ui->groupBox_plane->setChecked(activeImageData->showPlane);
		this->ui->planeOrientationComboBox->setCurrentIndex(activeImageData->orientIndex);
		this->ui->planeMedianCheckBox->setChecked(activeImageData->checkMedian);
		this->ui->kernelXSpinBox->setMaximum(activeImageData->VOI[1]);
		this->ui->kernelYSpinBox->setMaximum(activeImageData->VOI[3]);
		this->ui->kernelZSpinBox->setMaximum(activeImageData->VOI[5]);
		this->ui->kernelXSpinBox->setValue(activeImageData->medianKernelX);
		this->ui->kernelYSpinBox->setValue(activeImageData->medianKernelY);
		this->ui->kernelZSpinBox->setValue(activeImageData->medianKernelZ);
		this->ui->checkBox_flipPlane->setChecked(activeImageData->switchPlane);
		this->ui->planVisibilityCheckBox->setChecked(activeImageData->planeIsVisible);
		//data infos
		const QFileInfo activeFile(activeImageData->fileName);
		statusLabel->setText(tr("Active dataset: ") + activeFile.fileName());

		fitSelectedAction->setEnabled(true);
		fitAllAction->setEnabled(true);
		updateRangePresentation();
		updateMetadata();
		setRangesPending(false);
	}else{
		QMessageBox::information(this,tr("ERROR"), tr("No valid Data selected"));
	}
}
void OCTview3R::slotExit()
{
	close();
}

void OCTview3R::slotShowAbout()
{
	AboutDialog dialog(this);
	dialog.exec();
}

void OCTview3R::slotShowObject(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->showObject = value;
		activeImageData->visibilityDirty = true;
	refreshViewer();
		if(ui->actionObject->isChecked() != value)
			ui->actionObject->setChecked(value);
		if(ui->groupBox_object->isChecked() != value)
			ui->groupBox_object->setChecked(value);
	}
}
void OCTview3R::slotShowPlane(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr &&
	   activeImageData->fileLoaded && activeImageData->isVolume){
		activeImageData->changePlaneInput = true;
		activeImageData->planeDirty = true;
		if(value){
			activeImageData->showPlane = true;
			onePlaneCallbackMutex = true;
			if (activeImageData->planeObserverTag == 0)
			{
				vtkSmartPointer<vtkCallbackCommand> startCallback = vtkSmartPointer<vtkCallbackCommand>::New();
				startCallback->SetCallback(onePlaneCallbackFunction);
				startCallback->SetClientData(this);
				activeImageData->planeObserverTag =
					activeImageData->planeWidget->AddObserver(vtkCommand::InteractionEvent, startCallback);
			}
		}else{
			onePlaneCallbackMutex = false;
			activeImageData->showPlane = false;
			if (activeImageData->planeObserverTag != 0)
			{
				activeImageData->planeWidget->RemoveObserver(activeImageData->planeObserverTag);
				activeImageData->planeObserverTag = 0;
			}
		}
	refreshViewer();
		if(ui->actionPlane->isChecked() != value)
			ui->actionPlane->setChecked(value);
		if(ui->groupBox_plane->isChecked() != value)
			ui->groupBox_plane->setChecked(value);
	}
}
void OCTview3R::onePlaneCallbackFunction(vtkObject* caller, unsigned long eventId, void *clientData, void *callData)
{
	OCTview3R *self = reinterpret_cast<OCTview3R*>(clientData);
	if(self->onePlaneCallbackMutex){
		self->onePlaneCallbackMutex = false;
		// TODO: reset plane outline on release middle mouse button (vtkCommand::EndInteractionEvent)
		self->markPlaneDirty();
	self->refreshViewer();
		self->onePlaneCallbackMutex = true;
	}
}
void OCTview3R::slotCheckFlipPlane(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->switchPlane = value;
		markPlaneDirty();
	refreshViewer();
	}
}

void OCTview3R::slotSetThreshold()
{
	if(settings.oneFileLoaded && activeImageData != nullptr &&
	   activeImageData->fileLoaded && activeImageData->isVolume){
		activeImageData->currentMinThreshold = this->ui->Slider_minThreshold->value();
		activeImageData->currentMaxThreshold = this->ui->Slider_maxThreshold->value();
		if (activeImageData->adjustColormap)
			applyAutomaticWindowLevel();
		markAppearanceDirty();
		markDataPipelineDirty();
	refreshViewer();
	}
}
void OCTview3R::slotCheckMinThresholdSlider(int value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		if(value >= activeImageData->currentMaxThreshold){
			this->ui->Slider_minThreshold->setValue(activeImageData->currentMaxThreshold);
		}
	}
}
void OCTview3R::slotCheckMaxThresholdSlider(int value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		if(value <= activeImageData->currentMinThreshold){
			this->ui->Slider_maxThreshold->setValue(activeImageData->currentMinThreshold);
		}
	}
}
void OCTview3R::slotOpenDataFileDialog()
{
	openData->showDialog();
}
void OCTview3R::slotOpenPolyFileDialog()
{
	openPoly->showDialog();
}
void OCTview3R::slotDataFileDialogClosed(
	vtkImageData* tmpData,
	unsigned int spacingInMillimetresMask)
{
	openData->setEnabled(true);
	if(openData->isValidData() && (tmpData != nullptr)){
		const bool hadLoadedData = !documentModel.empty();
		ImageData* data = &documentModel.create();
		data->showObject	= true;
		data->isVolume		= true;
		data->isPolyData	= false;
		data->typeName		= "Volume";
		data->fileName		= openData->getFileName();
		data->filePath		= openData->getFilePath();
		data->dataFormat	= openData->getDataFormat();
		int dimensions[3] = { 0, 0, 0 };
		int extent[6] = { 0, 0, 0, 0, 0, 0 };
		tmpData->GetDimensions(dimensions);
		tmpData->GetExtent(extent);
		data->width			= dimensions[0];
		data->height		= dimensions[1];
		data->depth			= dimensions[2];
		data->bitsize		= tmpData->GetScalarSize() > 1 ? bitsizeType::BIT16 : bitsizeType::BIT8;
		data->endian		= openData->getEndian();
		double scalarRange[2] = { 0.0, 0.0 };
		tmpData->GetScalarRange(scalarRange);
		data->minValue		= static_cast<int>(std::floor(scalarRange[0]));
		data->maxValue		= static_cast<int>(std::ceil(scalarRange[1]));
		data->currentMinThreshold = data->minValue;
		data->currentMaxThreshold = data->maxValue;
		data->renderRgb = tmpData->GetNumberOfScalarComponents() >= 3;
		data->windowWidth = std::max(
			1,
			data->maxValue - data->minValue + 1);
		data->windowLevel = static_cast<int>(std::round(
			0.5 * (static_cast<double>(data->minValue) +
				data->maxValue + 1.0)));
		data->VOI[0]		= static_cast<double>(extent[0]);
		data->VOI[1]		= static_cast<double>(extent[1] + 1);
		data->VOI[2]		= static_cast<double>(extent[2]);
		data->VOI[3]		= static_cast<double>(extent[3] + 1);
		data->VOI[4]		= static_cast<double>(extent[4]);
		data->VOI[5]		= static_cast<double>(extent[5] + 1);
		for (int i = 0; i < 6; ++i)
			data->sourceVOI[i] = data->VOI[i];
		tmpData->GetSpacing(data->spacing);
		data->spacingInMillimetresMask = spacingInMillimetresMask;
		data->pointSize		= 1;
		data->fileLoaded	= true;
		data->poly			= nullptr;
		data->image			= tmpData;
		tmpData->Delete();

		// Transfer ownership to the document model and add a matching tab.
		addDocumentTab(*data);
		settings.oneFileLoaded = true;
		//set values in GUI
		slotSetImageData(data);
		//render all
		if(!hadLoadedData)
			settings.firstFileLoaded = true;
	refreshViewer();
	}else if (tmpData != nullptr){
		tmpData->Delete();
	}
}
void OCTview3R::slotPolyFileDialogClosed(vtkPolyData* tmpPoly)
{
	openPoly->setEnabled(true);
	if(openPoly->isValidData() && (tmpPoly != nullptr)){
		const bool hadLoadedData = !documentModel.empty();
		ImageData* data = &documentModel.create();
		data->showObject	= true;
		data->isVolume		= false;
		data->isPolyData	= true;
		data->typeName		= "PolyData";
		data->fileName		= openPoly->getFileName();
		data->filePath		= openPoly->getFilePath();
		data->polyFormat	= openPoly->getPolyFormat();
		tmpPoly->GetBounds(data->VOI);
		for (int i = 0; i < 6; ++i)
			data->sourceVOI[i] = data->VOI[i];
		data->fileLoaded	= true;
		data->poly			= tmpPoly;
		data->image			= nullptr;
		data->polyMode		=
			tmpPoly->GetNumberOfPolys() > 0 ||
			tmpPoly->GetNumberOfStrips() > 0
				? 2
				: 0;
		tmpPoly->Delete();
		
		// Transfer ownership to the document model and add a matching tab.
		addDocumentTab(*data);
		settings.oneFileLoaded = true;
		//set values in GUI
		slotSetImageData(data);
		//render all
		if(!hadLoadedData)
			settings.firstFileLoaded = true;
	refreshViewer();
	}else if (tmpPoly != nullptr){
		tmpPoly->Delete();
	}
}
void OCTview3R::slotProcessDataFile()
{
	if(openData->isValidData()){
		openData->setLoading(true);
		loading *worker = new loading(openData);
		QThread *thread = new QThread(this);
		loadingThreads.append(thread);
		worker->moveToThread(thread);
		connect(worker, &loading::updateProgress, openData, &OpenData::updateProgress);
		connect(worker, &loading::dataLoaded, this, &OCTview3R::slotDataFileDialogClosed);
		connect(worker, &loading::dataLoaded, openData, &OpenData::doAccepted);
		connect(worker, &loading::failed, this, &OCTview3R::slotDataLoadFailed);
		connect(worker, &loading::finished, worker, &QObject::deleteLater);
		connect(worker, &loading::finished, thread, &QThread::quit);
		connect(thread, &QThread::finished, this, [this, thread]() {
			loadingThreads.removeOne(thread);
		});
		connect(thread, &QThread::finished, thread, &QObject::deleteLater);
		connect(thread, &QThread::started, worker, &loading::loadData);
		thread->start();
	}else{
		QMessageBox::information(this,tr("ERROR"), tr("No valid data selected"));
	}
}
void OCTview3R::slotProcessPolyFile()
{
	if(openPoly->isValidData()){
		openPoly->setLoading(true);
		loading *worker = new loading(openPoly);
		QThread *thread = new QThread(this);
		loadingThreads.append(thread);
		worker->moveToThread(thread);
		connect(worker, &loading::updateProgress, openPoly, &OpenPoly::updateProgress);
		connect(worker, &loading::polyLoaded, this, &OCTview3R::slotPolyFileDialogClosed);
		connect(worker, &loading::polyLoaded, openPoly, &OpenPoly::doAccepted);
		connect(worker, &loading::failed, this, &OCTview3R::slotPolyLoadFailed);
		connect(worker, &loading::finished, worker, &QObject::deleteLater);
		connect(worker, &loading::finished, thread, &QThread::quit);
		connect(thread, &QThread::finished, this, [this, thread]() {
			loadingThreads.removeOne(thread);
		});
		connect(thread, &QThread::finished, thread, &QObject::deleteLater);
		connect(thread, &QThread::started, worker, &loading::loadPoly);
		thread->start();
	}else{
		QMessageBox::information(this,tr("ERROR"), tr("No valid data selected"));
	}
}

void OCTview3R::slotDataLoadFailed(const QString& message)
{
	openData->setLoading(false);
	QMessageBox::critical(this, tr("Volume loading failed"), message);
}

void OCTview3R::slotPolyLoadFailed(const QString& message)
{
	openPoly->setLoading(false);
	QMessageBox::critical(this, tr("Polygonal-data loading failed"), message);
}

void OCTview3R::refreshViewer()
{
	if (!viewerController)
		return;

	const bool resetCamera = settings.firstFileLoaded;
	if (resetCamera)
	{
		settings.firstFileLoaded = false;
		cam = viewerController->renderer()->GetActiveCamera();
		cam->SetFocalPoint(0.0, 0.0, 0.0);
		cam->SetViewUp(1.0, 0.0, 0.0);
		cam->SetPosition(0.0, 0.0, -1.0);
	}
	viewerController->refresh(
		documentModel,
		activeImageData,
		settings,
		resetCamera);
}

void OCTview3R::slotSetColormap(QString value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->colormapName = value;
		markAppearanceDirty();
		refreshViewer();
	}
}
void OCTview3R::slotPickVolumeColor()
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		QColor color = QColorDialog::getColor(activeImageData->volumeColor, this);
		if(color.isValid()){
			activeImageData->volumeColor = color;
			this->ui->pushButton_pickVolumeColor->setStyleSheet("background-color: "+activeImageData->volumeColor.name());
			markAppearanceDirty();
	refreshViewer();
		}
	}
}
void OCTview3R::slotPickPolyColor()
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		QColor color = QColorDialog::getColor(activeImageData->polyColor, this);
		if(color.isValid()){
			activeImageData->polyColor = color;
			this->ui->pushButton_pickPolyColor->setStyleSheet("background-color: "+activeImageData->polyColor.name());
			markAppearanceDirty();
	refreshViewer();
		}
	}
}
void OCTview3R::slotShowScalarBar(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		if(value){
			settings.showScalarBar= true;
			this->ui->actionScalarBar->setChecked(true);
		}else{
			settings.showScalarBar = false;
			this->ui->actionScalarBar->setChecked(false);
		}
		viewerController->refreshDecorations(documentModel, activeImageData, settings);
	}
}
void OCTview3R::slotShowAxesTriad(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		if(value){
			settings.showAxesTriad = true;
			this->ui->actionAxesTriad->setChecked(true);
			settings.showAxesBox = false;
			this->ui->actionAxesBox->setChecked(false);
		}else{
			settings.showAxesTriad = false;
			this->ui->actionAxesTriad->setChecked(false);
		}
		viewerController->refreshDecorations(documentModel, activeImageData, settings);
	}
}
void OCTview3R::slotShowOrientAxes(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		if(value){
			settings.showOrientAxes = true;
			this->ui->actionOrientAxes->setChecked(true);
		}else{
			settings.showOrientAxes = false;
			this->ui->actionOrientAxes->setChecked(false);
		}
		viewerController->refreshDecorations(documentModel, activeImageData, settings);
	}
}
void OCTview3R::slotShowAxesBox(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		if(value){
			settings.showAxesBox = true;
			this->ui->actionAxesBox->setChecked(true);
			settings.showAxesTriad = false;
			this->ui->actionAxesTriad->setChecked(false);
		}else{
			settings.showAxesBox = false;
			this->ui->actionAxesBox->setChecked(false);
		}
		viewerController->refreshDecorations(documentModel, activeImageData, settings);
	}
}
void OCTview3R::slotAdjustColormap(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->adjustColormap = value;
		if (value && activeImageData->isVolume)
			applyAutomaticWindowLevel();
		markAppearanceDirty();
		if (activeImageData->renderRgb)
			markDataPipelineDirty();
	refreshViewer();
	}
}
void OCTview3R::slotInvertColormap(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->invertColormap = value;
		markAppearanceDirty();
	refreshViewer();
	}
}
void OCTview3R::slotMedianCheckBox(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr &&
	   activeImageData->fileLoaded && activeImageData->isVolume){
		activeImageData->checkMedian = value;
		activeImageData->changePlaneInput = true;
		markPlaneDirty();
	refreshViewer();
	}
}
void OCTview3R::slotKernelXChanged(int value)
{
	if(!settings.oneFileLoaded || activeImageData == nullptr || !activeImageData->fileLoaded)
		return;
	activeImageData->medianKernelX = value;
	slotMedianCheckBox(activeImageData->checkMedian);
}
void OCTview3R::slotKernelYChanged(int value)
{
	if(!settings.oneFileLoaded || activeImageData == nullptr || !activeImageData->fileLoaded)
		return;
	activeImageData->medianKernelY = value;
	slotMedianCheckBox(activeImageData->checkMedian);
}
void OCTview3R::slotKernelZChanged(int value)
{
	if(!settings.oneFileLoaded || activeImageData == nullptr || !activeImageData->fileLoaded)
		return;
	activeImageData->medianKernelZ = value;
	slotMedianCheckBox(activeImageData->checkMedian);
}
void OCTview3R::slotOrientationChanged(int value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr &&
	   activeImageData->fileLoaded && activeImageData->isVolume){
		activeImageData->changePlaneInput = true;
		activeImageData->orientChanged = true;
		activeImageData->orientIndex = value;
		markPlaneDirty();
	refreshViewer();
	}
}
void OCTview3R::slotGetPlaneData()
{
	if(settings.oneFileLoaded && activeImageData != nullptr &&
	   activeImageData->fileLoaded && activeImageData->isVolume){
		double worldCenter[3] = {};
		double worldNormal[3] = {};
		activeImageData->transform->TransformPoint(
			activeImageData->planeWidget->GetCenter(), worldCenter);
		activeImageData->transform->TransformNormal(
			activeImageData->planeWidget->GetNormal(), worldNormal);
		this->ui->planeLineEdit->setText(
			"normal:" + QString::number(worldNormal[0], 'g', 4) + ":" +
			QString::number(worldNormal[1], 'g', 4) + ":" +
			QString::number(worldNormal[2], 'g', 4) +
			" | center:" + QString::number(worldCenter[0], 'g', 4) + ":" +
			QString::number(worldCenter[1], 'g', 4) + ":" +
			QString::number(worldCenter[2], 'g', 4));
	}else{
		this->ui->planeLineEdit->clear();
	}
}
void OCTview3R::slotRotX(double value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->rot[0] = value;
		markTransformDirty();
	refreshViewer();
	}
}
void OCTview3R::slotRotY(double value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->rot[1] = value;
		markTransformDirty();
	refreshViewer();
	}
}
void OCTview3R::slotRotZ(double value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->rot[2] = value;
		markTransformDirty();
	refreshViewer();
	}
}
void OCTview3R::slotShiftX(double value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->shift[0] = value;
		markTransformDirty();
	refreshViewer();
	}
}
void OCTview3R::slotShiftY(double value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->shift[1] = value;
		markTransformDirty();
	refreshViewer();
	}
}
void OCTview3R::slotShiftZ(double value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->shift[2] = value;
		markTransformDirty();
	refreshViewer();
	}
}
void OCTview3R::slotApplyRanges()
{
	if(!settings.oneFileLoaded || activeImageData == nullptr ||
	   !activeImageData->fileLoaded ||
	   (!activeImageData->isVolume && !activeImageData->isPolyData))
		return;

	const double requestedDisplayRange[6] = {
		ui->x0DoubleSpinBox->value(), ui->x1DoubleSpinBox->value(),
		ui->y0DoubleSpinBox->value(), ui->y1DoubleSpinBox->value(),
		ui->z0DoubleSpinBox->value(), ui->z1DoubleSpinBox->value()
	};
	double requestedVOI[6] = {};
	for (int axis = 0; axis < 3; ++axis)
	{
		requestedVOI[2 * axis] = cropVoxelIndex(
			*activeImageData, axis, requestedDisplayRange[2 * axis]);
		requestedVOI[2 * axis + 1] = cropVoxelIndex(
			*activeImageData, axis, requestedDisplayRange[2 * axis + 1]);
	}
	const bool ordered =
		requestedVOI[0] < requestedVOI[1] &&
		requestedVOI[2] < requestedVOI[3] &&
		requestedVOI[4] < requestedVOI[5];
	const double boundaryTolerance =
		activeImageData->isPolyData ? 0.0001 : 0.0;
	bool insideSource = true;
	for (int i = 0; i < 6; i += 2)
	{
		insideSource = insideSource &&
			requestedVOI[i] >=
				activeImageData->sourceVOI[i] - boundaryTolerance &&
			requestedVOI[i + 1] <=
				activeImageData->sourceVOI[i + 1] + boundaryTolerance;
	}
	if (!ordered || !insideSource)
	{
		QMessageBox::information(
			this,
			tr("Invalid ranges"),
			tr("Each range must contain at least one voxel after rounding and remain inside the source extent."));
		return;
	}

	for (int i = 0; i < 6; i += 2)
	{
		activeImageData->VOI[i] = std::max(
			requestedVOI[i],
			activeImageData->sourceVOI[i]);
		activeImageData->VOI[i + 1] = std::min(
			requestedVOI[i + 1],
			activeImageData->sourceVOI[i + 1]);
	}
	// Reflect the actual voxel boundaries selected after rounding back in the
	// micrometre controls so the displayed crop always matches VTK's VOI.
	QDoubleSpinBox* rangeControls[6] = {
		ui->x0DoubleSpinBox, ui->x1DoubleSpinBox,
		ui->y0DoubleSpinBox, ui->y1DoubleSpinBox,
		ui->z0DoubleSpinBox, ui->z1DoubleSpinBox
	};
	std::vector<std::unique_ptr<QSignalBlocker>> rangeBlockers;
	for (QDoubleSpinBox* control : rangeControls)
		rangeBlockers.emplace_back(std::make_unique<QSignalBlocker>(control));
	for (int axis = 0; axis < 3; ++axis)
	{
		rangeControls[2 * axis]->setValue(cropDisplayValue(
			*activeImageData, axis, activeImageData->VOI[2 * axis]));
		rangeControls[2 * axis + 1]->setValue(cropDisplayValue(
			*activeImageData, axis, activeImageData->VOI[2 * axis + 1]));
	}
	if (activeImageData->isVolume)
	{
		activeImageData->changePlaneInput = true;
		activeImageData->orientChanged = true;
		markPlaneDirty();
	}
	markDataPipelineDirty();
	setRangesPending(false);
	updateMetadata();
	refreshViewer();
}

void OCTview3R::slotRangesEdited()
{
	if (!activeImageData || !activeImageData->fileLoaded)
		return;
	const double values[6] = {
		ui->x0DoubleSpinBox->value(), ui->x1DoubleSpinBox->value(),
		ui->y0DoubleSpinBox->value(), ui->y1DoubleSpinBox->value(),
		ui->z0DoubleSpinBox->value(), ui->z1DoubleSpinBox->value()
	};
	bool changed = false;
	for (int axis = 0; axis < 3; ++axis)
	{
		for (int side = 0; side < 2; ++side)
		{
			const int index = 2 * axis + side;
			const double currentDisplayValue = cropDisplayValue(
				*activeImageData, axis, activeImageData->VOI[index]);
			const int decimals = cropAxisIsCalibrated(*activeImageData, axis)
				? 2
				: (activeImageData->isPolyData ? 4 : 0);
			const double displayTolerance =
				0.5 * std::pow(10.0, -decimals) + 1e-12;
			changed = changed ||
				std::abs(values[index] - currentDisplayValue) > displayTolerance;
		}
	}
	setRangesPending(changed);
}

void OCTview3R::slotResetRanges()
{
	if (!activeImageData || !activeImageData->fileLoaded)
		return;
	QDoubleSpinBox* rangeControls[6] = {
		ui->x0DoubleSpinBox, ui->x1DoubleSpinBox,
		ui->y0DoubleSpinBox, ui->y1DoubleSpinBox,
		ui->z0DoubleSpinBox, ui->z1DoubleSpinBox
	};
	for (int axis = 0; axis < 3; ++axis)
	{
		rangeControls[2 * axis]->setValue(cropDisplayValue(
			*activeImageData, axis, activeImageData->sourceVOI[2 * axis]));
		rangeControls[2 * axis + 1]->setValue(cropDisplayValue(
			*activeImageData, axis, activeImageData->sourceVOI[2 * axis + 1]));
	}
	slotApplyRanges();
}

void OCTview3R::slotResetObjectTransform()
{
	if (!activeImageData || !activeImageData->fileLoaded)
		return;
	QDoubleSpinBox* spinBoxes[] = {
		ui->rotXDoubleSpinBox, ui->rotYDoubleSpinBox, ui->rotZDoubleSpinBox,
		ui->shiftXDoubleSpinBox, ui->shiftYDoubleSpinBox, ui->shiftZDoubleSpinBox,
		ui->scaleXDoubleSpinBox, ui->scaleYDoubleSpinBox, ui->scaleZDoubleSpinBox
	};
	std::vector<std::unique_ptr<QSignalBlocker>> blockers;
	for (QDoubleSpinBox* spinBox : spinBoxes)
		blockers.emplace_back(std::make_unique<QSignalBlocker>(spinBox));
	for (int axis = 0; axis < 3; ++axis)
	{
		activeImageData->rot[axis] = 0.0;
		activeImageData->shift[axis] = 0.0;
		activeImageData->scale[axis] = 1.0;
		spinBoxes[axis]->setValue(0.0);
		spinBoxes[axis + 3]->setValue(0.0);
		spinBoxes[axis + 6]->setValue(1.0);
	}
	markTransformDirty();
	refreshViewer();
}
void OCTview3R::slotPlaneUp()
{
	if(settings.oneFileLoaded && activeImageData != nullptr &&
	   activeImageData->fileLoaded && activeImageData->isVolume){
		double aNormal[3]; 
		double *normal = aNormal; 
		normal = activeImageData->planeWidget->GetNormal();
		const bool isOrthogonal =
			std::abs(std::abs(normal[0]) - 1.0) < 1e-6 ||
			std::abs(std::abs(normal[1]) - 1.0) < 1e-6 ||
			std::abs(std::abs(normal[2]) - 1.0) < 1e-6;
		if(isOrthogonal){
			int index = activeImageData->planeWidget->GetSliceIndex();
			activeImageData->planeWidget->SetSliceIndex(index + 1);
			markPlaneDirty();
			refreshViewer();
		}else{
			QMessageBox::information(this,tr("WARNING"), tr("Plane not orthogonal to x, y or z."));	
		}
		slotGetPlaneData();
	}
}
void OCTview3R::slotPlaneDown()
{
	if(settings.oneFileLoaded && activeImageData != nullptr &&
	   activeImageData->fileLoaded && activeImageData->isVolume){
		double aNormal[3]; 
		double *normal = aNormal; 
		normal = activeImageData->planeWidget->GetNormal();
		const bool isOrthogonal =
			std::abs(std::abs(normal[0]) - 1.0) < 1e-6 ||
			std::abs(std::abs(normal[1]) - 1.0) < 1e-6 ||
			std::abs(std::abs(normal[2]) - 1.0) < 1e-6;
		if(isOrthogonal){
			int index = activeImageData->planeWidget->GetSliceIndex();
			activeImageData->planeWidget->SetSliceIndex(index - 1);
			markPlaneDirty();
			refreshViewer();
		}else{
			QMessageBox::information(this,tr("WARNING"), tr("Plane not orthogonal to x, y or z."));	
		}
		slotGetPlaneData();
	}
}

void OCTview3R::slotSetPointSize(int value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->pointSize = value;
		markAppearanceDirty();
	refreshViewer();
	}
}

void OCTview3R::slotSetObjectOpacity(int value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->objectOpacity = (double)value * 0.01;
		markAppearanceDirty();
	refreshViewer();
	}
}

void OCTview3R::slotSetWindowWidth(int value)
{
	if (!settings.oneFileLoaded || activeImageData == nullptr ||
		!activeImageData->fileLoaded || !activeImageData->isVolume)
	{
		return;
	}
	activeImageData->windowWidth = std::max(1, value);
	activeImageData->adjustColormap = false;
	{
		const QSignalBlocker blocker(ui->checkBox_adjustColormap);
		ui->checkBox_adjustColormap->setChecked(false);
	}
	markAppearanceDirty();
	if (activeImageData->renderRgb)
		markDataPipelineDirty();
	refreshViewer();
}

void OCTview3R::slotSetWindowLevel(int value)
{
	if (!settings.oneFileLoaded || activeImageData == nullptr ||
		!activeImageData->fileLoaded || !activeImageData->isVolume)
	{
		return;
	}
	activeImageData->windowLevel = value;
	activeImageData->adjustColormap = false;
	{
		const QSignalBlocker blocker(ui->checkBox_adjustColormap);
		ui->checkBox_adjustColormap->setChecked(false);
	}
	markAppearanceDirty();
	if (activeImageData->renderRgb)
		markDataPipelineDirty();
	refreshViewer();
}

void OCTview3R::slotSetPolyGloss(int value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr &&
	   activeImageData->fileLoaded &&
	   activeImageData->isPolyData){
		activeImageData->polyGloss = static_cast<double>(value) * 0.01;
		markAppearanceDirty();
	refreshViewer();
	}
}

void OCTview3R::slotSetBlendMode(int index)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->blendMode = index;
		markAppearanceDirty();
	refreshViewer();
	}
}
void OCTview3R::slotSetColorMode(int index)
{
	if (!settings.oneFileLoaded || activeImageData == nullptr ||
		!activeImageData->fileLoaded || !activeImageData->isVolume ||
		activeImageData->image == nullptr)
	{
		return;
	}

	const bool rgbAvailable =
		activeImageData->image->GetNumberOfScalarComponents() >= 3;
	activeImageData->renderRgb = rgbAvailable && index == 1;
	activeImageData->changePlaneInput = true;
	updateColorModeControls();
	updateMetadata();
	markAppearanceDirty();
	markDataPipelineDirty();
	markPlaneDirty();
	refreshViewer();
}
void OCTview3R::slotSetPolyMode(int index)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->polyMode = index;
		markAppearanceDirty();
	refreshViewer();
	}
}
void OCTview3R::slotPlaneVisibility(bool value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->planeIsVisible = value;
		markPlaneDirty();
	refreshViewer();
	}
}
void OCTview3R::slotFrontX()
{
	cam = viewerController->renderer()->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(0,1,0);
	cam->SetPosition(-1,0,0);
	slotResetCam();
}
void OCTview3R::slotFrontY()
{
	cam = viewerController->renderer()->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(0,0,1);
	cam->SetPosition(0,-1,0);
	slotResetCam();
}
void OCTview3R::slotFrontZ()
{
	cam = viewerController->renderer()->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(1,0,0);
	cam->SetPosition(0,0,-1);
	slotResetCam();
}
void OCTview3R::slotBackX()
{
	cam = viewerController->renderer()->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(0,1,0);
	cam->SetPosition(1,0,0);
	slotResetCam();
}
void OCTview3R::slotBackY()
{
	cam = viewerController->renderer()->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);	
	cam->SetViewUp(0,0,1);
	cam->SetPosition(0,1,0);
	slotResetCam();
}
void OCTview3R::slotBackZ()
{
	cam = viewerController->renderer()->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(1,0,0);
	cam->SetPosition(0,0,1);
	slotResetCam();
}
void OCTview3R::slotRot90()
{
	cam = viewerController->renderer()->GetActiveCamera();
	double roll = cam->GetRoll();
	cam->SetRoll(roll - 90.0);
	slotResetCam();
}
void OCTview3R::slotFitSelected()
{
	if (activeImageData && activeImageData->fileLoaded)
		viewerController->fitToDocument(*activeImageData);
}
void OCTview3R::slotFitAll()
{
	if (!documentModel.empty())
		viewerController->fitAll();
}
void OCTview3R::slotParallelProjection(bool enabled)
{
	if (viewerController)
		viewerController->setParallelProjection(enabled);
}
void OCTview3R::slotScaleX(double value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->scale[0] = value;
		markTransformDirty();
		refreshViewer();
	}
}
void OCTview3R::slotScaleY(double value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->scale[1] = value;
		markTransformDirty();
		refreshViewer();
	}
}
void OCTview3R::slotScaleZ(double value)
{
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		activeImageData->scale[2] = value;
		markTransformDirty();
		refreshViewer();
	}
}
void OCTview3R::slotRotCamX(double value)
{
	double dx = value - settings.x_rot_cam;
	settings.x_rot_cam = value;
	cam = viewerController->renderer()->GetActiveCamera();
	double focalPoint[3] = { 0.0, 0.0, 0.0 };
	cam->GetFocalPoint(focalPoint);
	camTrans->Identity();
	camTrans->PreMultiply();
	camTrans->Translate(focalPoint);
	camTrans->RotateX(dx);
	camTrans->Translate(-focalPoint[0], -focalPoint[1], -focalPoint[2]);
	cam->ApplyTransform(camTrans);
	cam->OrthogonalizeViewUp();
	viewerController->renderer()->ResetCameraClippingRange();
	viewerController->render();
}
void OCTview3R::slotRotCamY(double value)
{
	double dy = value - settings.y_rot_cam;
	settings.y_rot_cam = value;
	cam = viewerController->renderer()->GetActiveCamera();
	double focalPoint[3] = { 0.0, 0.0, 0.0 };
	cam->GetFocalPoint(focalPoint);
	camTrans->Identity();
	camTrans->PreMultiply();
	camTrans->Translate(focalPoint);
	camTrans->RotateY(dy);
	camTrans->Translate(-focalPoint[0], -focalPoint[1], -focalPoint[2]);
	cam->ApplyTransform(camTrans);
	cam->OrthogonalizeViewUp();
	viewerController->renderer()->ResetCameraClippingRange();
	viewerController->render();
}
void OCTview3R::slotRotCamZ(double value)
{
	double dz = value - settings.z_rot_cam;
	settings.z_rot_cam = value;
	cam = viewerController->renderer()->GetActiveCamera();
	double focalPoint[3] = { 0.0, 0.0, 0.0 };
	cam->GetFocalPoint(focalPoint);
	camTrans->Identity();
	camTrans->PreMultiply();
	camTrans->Translate(focalPoint);
	camTrans->RotateZ(dz);
	camTrans->Translate(-focalPoint[0], -focalPoint[1], -focalPoint[2]);
	cam->ApplyTransform(camTrans);
	cam->OrthogonalizeViewUp();
	viewerController->renderer()->ResetCameraClippingRange();
	viewerController->render();
}
void OCTview3R::slotRotCamStepX(double value)
{
	this->ui->rotXCamDoubleSpinBox->setSingleStep(value);
}
void OCTview3R::slotRotCamStepY(double value)
{
	this->ui->rotYCamDoubleSpinBox->setSingleStep(value);
}
void OCTview3R::slotRotCamStepZ(double value)
{	
	this->ui->rotZCamDoubleSpinBox->setSingleStep(value);
}
void OCTview3R::slotBackground1(QString qstr)
{
	const QStringList components = qstr.split("-");
	if (components.size() != 3)
		return;
	settings.background_RGB1[0] = components[0].toInt();
	settings.background_RGB1[1] = components[1].toInt();
	settings.background_RGB1[2] = components[2].toInt();
	viewerController->setBackground1(
		settings.background_RGB1[0],
		settings.background_RGB1[1],
		settings.background_RGB1[2]);
	viewerController->render();
}
void OCTview3R::slotBackground2(QString qstr)
{
	const QStringList components = qstr.split("-");
	if (components.size() != 3)
		return;
	settings.background_RGB2[0] = components[0].toInt();
	settings.background_RGB2[1] = components[1].toInt();
	settings.background_RGB2[2] = components[2].toInt();
	viewerController->setBackground2(
		settings.background_RGB2[0],
		settings.background_RGB2[1],
		settings.background_RGB2[2]);
	viewerController->render();
}
void OCTview3R::slotSaveDisplay()
{
	auto w2i = vtkSmartPointer<vtkWindowToImageFilter>::New();
	auto writer = vtkSmartPointer<vtkTIFFWriter>::New();
	w2i->SetInput(viewerController->renderWindow());
	w2i->Update();
	writer->SetInputConnection(w2i->GetOutputPort());
	QFileDialog getFileDialog(this, "Save display as...");
	getFileDialog.setNameFilter("Image file (*.tif)");
	getFileDialog.setAcceptMode(QFileDialog::AcceptSave);	
	if(getFileDialog.exec() == QFileDialog::Accepted){
		writer->SetFileName(getFileDialog.selectedFiles()[0].toStdString().c_str());
		viewerController->render();
		writer->Write();
	}else{
		QMessageBox::information(this,tr("WARNING"), tr("No image was saved"));	
	}
}
void OCTview3R::slotCloseTab()
{
	slotCloseTab(ui->tabWidget->currentIndex());
}

void OCTview3R::slotCloseTab(int index)
{
	if (index < 0 || index >= documentModel.size())
		return;

	DocumentModel::Document data = documentModel.takeAt(index);
	viewerController->detach(*data);

	const QSignalBlocker tabBlocker(ui->tabWidget);
	QWidget* page = ui->tabWidget->widget(index);
	ui->tabWidget->removeTab(index);
	if (page)
		page->deleteLater();
	settings.oneFileLoaded = !documentModel.empty();
	activeImageData = documentModel.active();

	if (documentModel.empty())
	{
		ui->tabWidget->addTab(new QWidget(), tr("Open File"));
		ui->tabWidget->setTabsClosable(false);
		ui->pushButton_render->setEnabled(false);
		ui->groupBox_object->setEnabled(false);
		ui->groupBox_plane->setEnabled(false);
		ui->groupBox_threshold->setEnabled(false);
		ui->groupBox_colorMapping->setEnabled(false);
		ui->actionObject->setCheckable(false);
		ui->actionPlane->setCheckable(false);
		ui->actionScalarBar->setCheckable(false);
		ui->actionAxesBox->setCheckable(false);
		ui->actionAxesTriad->setCheckable(false);
		ui->actionOrientAxes->setCheckable(false);
		statusLabel->setText(tr("No file loaded."));
		fitSelectedAction->setEnabled(false);
		fitAllAction->setEnabled(false);
		updateMetadata();
		updateRangePresentation();
		setRangesPending(false);
		viewerController->refresh(documentModel, nullptr, settings);
		return;
	}

	const int nextIndex = qMin(index, documentModel.size() - 1);
	ui->tabWidget->setCurrentIndex(nextIndex);
	slotSetImageData(nextIndex);
}
