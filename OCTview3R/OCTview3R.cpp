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
#include <QFileDialog>
#include <QColorDialog>
#include <QLabel>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QThread>
#include <qfileinfo.h>

#include <cmath>

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
	settings.activeIndex			= 0;
	settings.numMaxIndex			= 0;
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

	settings.x_fac					= 1.0;
	settings.y_fac					= 1.0;
	settings.z_fac					= 1.0;

	settings.x_rot_cam				= 0.0;
	settings.y_rot_cam				= 0.0;
	settings.z_rot_cam				= 0.0;

	onePlaneCallbackMutex			= false;

	//general
	camTrans					= vtkSmartPointer<vtkTransform>::New();
	viewerController			= std::make_unique<ViewerController>();

	//setup ui pointer
	this->ui->setupUi(this);
	this->showMaximized();

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
	this->ui->label_minThreshold->setText("");
	this->ui->label_maxThreshold->setText("");
	this->ui->planeLineEdit->setReadOnly(true);
	this->ui->xLabel->setText("");
	this->ui->yLabel->setText("");
	this->ui->zLabel->setText("");
	this->ui->label_objectOpacity->setText("");
	this->ui->xLabel->setText("[0,0]");
	this->ui->yLabel->setText("[0,0]");
	this->ui->zLabel->setText("[0,0]");

	//set up action signals and slots
	connect(this->ui->actionOpenData, SIGNAL(triggered()), this, SLOT(slotOpenDataFileDialog()));
	connect(this->ui->actionOpenPolyData, SIGNAL(triggered()), this, SLOT(slotOpenPolyFileDialog()));
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
	connect(this->ui->Slider_objectOpacity, SIGNAL(sliderMoved(int)), this, SLOT(slotSetObjectOpacity(int)));
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
	connect(this->ui->pushButton_close, SIGNAL(clicked()), this, SLOT(slotCloseTab()));

	openData = new OpenData(this);
	connect(openData, SIGNAL(signalStartProcess()), this, SLOT(slotProcessDataFile()));
	openPoly = new OpenPoly(this);
	connect(openPoly, SIGNAL(signalStartProcess()), this, SLOT(slotProcessPolyFile()));

	//setup statusBar
	statusLabel = new QLabel();
	this->ui->statusBar->addWidget(statusLabel);
	statusLabel->setText("No file loaded.");

	initializeVTKPipeline();
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
	if(data->fileLoaded){
		//set activeImageData to new data
		activeImageData = data;
		documentModel.setActiveIndex(documentModel.indexOf(data));

		//allow actions
		this->ui->actionScalarBar->setCheckable(true);
		this->ui->actionAxesBox->setCheckable(true);
		this->ui->actionAxesTriad->setCheckable(true);
		this->ui->actionOrientAxes->setCheckable(true);

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
		const bool rangesSupported = activeImageData->isVolume;
		this->ui->x0DoubleSpinBox->setEnabled(rangesSupported);
		this->ui->x1DoubleSpinBox->setEnabled(rangesSupported);
		this->ui->y0DoubleSpinBox->setEnabled(rangesSupported);
		this->ui->y1DoubleSpinBox->setEnabled(rangesSupported);
		this->ui->z0DoubleSpinBox->setEnabled(rangesSupported);
		this->ui->z1DoubleSpinBox->setEnabled(rangesSupported);
		this->ui->pushButton_render->setEnabled(rangesSupported);
		this->ui->x0DoubleSpinBox->setMinimum(activeImageData->sourceVOI[0]);
		this->ui->x0DoubleSpinBox->setMaximum(activeImageData->sourceVOI[1] - 1.0);
		this->ui->x0DoubleSpinBox->setValue(activeImageData->VOI[0]);
		this->ui->x1DoubleSpinBox->setMinimum(activeImageData->sourceVOI[0] + 1.0);
		this->ui->x1DoubleSpinBox->setMaximum(activeImageData->sourceVOI[1]);
		this->ui->x1DoubleSpinBox->setValue(activeImageData->VOI[1]);
		this->ui->y0DoubleSpinBox->setMinimum(activeImageData->sourceVOI[2]);
		this->ui->y0DoubleSpinBox->setMaximum(activeImageData->sourceVOI[3] - 1.0);
		this->ui->y0DoubleSpinBox->setValue(activeImageData->VOI[2]);
		this->ui->y1DoubleSpinBox->setMinimum(activeImageData->sourceVOI[2] + 1.0);
		this->ui->y1DoubleSpinBox->setMaximum(activeImageData->sourceVOI[3]);
		this->ui->y1DoubleSpinBox->setValue(activeImageData->VOI[3]);
		this->ui->z0DoubleSpinBox->setMinimum(activeImageData->sourceVOI[4]);
		this->ui->z0DoubleSpinBox->setMaximum(activeImageData->sourceVOI[5] - 1.0);
		this->ui->z0DoubleSpinBox->setValue(activeImageData->VOI[4]);
		this->ui->z1DoubleSpinBox->setMinimum(activeImageData->sourceVOI[4] + 1.0);
		this->ui->z1DoubleSpinBox->setMaximum(activeImageData->sourceVOI[5]);
		this->ui->z1DoubleSpinBox->setValue(activeImageData->VOI[5]);
		this->ui->xLabel->setText("["+QString().setNum(activeImageData->sourceVOI[0])+","+QString().setNum(activeImageData->sourceVOI[1])+"]");
		this->ui->yLabel->setText("["+QString().setNum(activeImageData->sourceVOI[2])+","+QString().setNum(activeImageData->sourceVOI[3])+"]");
		this->ui->zLabel->setText("["+QString().setNum(activeImageData->sourceVOI[4])+","+QString().setNum(activeImageData->sourceVOI[5])+"]");
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
		}
		//mark object change for rendering
		//activeImageData->fileChanged = true;

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
		this->ui->checkBox_adjustColormap->setChecked(activeImageData->adjustColormap);
		this->ui->checkBox_invertColormap->setChecked(activeImageData->invertColormap);

		//Set threshold values
		this->ui->groupBox_threshold->setEnabled(activeImageData->isVolume);
		this->ui->Slider_minThreshold->setMinimum(activeImageData->minValue);
		this->ui->Slider_minThreshold->setMaximum(activeImageData->maxValue);
		this->ui->Slider_minThreshold->setValue(activeImageData->currentMinThreshold);
		this->ui->Slider_minThreshold->setEnabled(activeImageData->isVolume);
		this->ui->label_minThreshold->setNum(activeImageData->currentMinThreshold);
		this->ui->Slider_maxThreshold->setMinimum(activeImageData->minValue);
		this->ui->Slider_maxThreshold->setMaximum(activeImageData->maxValue);
		this->ui->Slider_maxThreshold->setValue(activeImageData->currentMaxThreshold);
		this->ui->Slider_maxThreshold->setEnabled(activeImageData->isVolume);
		this->ui->label_maxThreshold->setNum(activeImageData->currentMaxThreshold);
		//mark threshold change for rendering
		activeImageData->changedThreshold = true;

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
		//mark plane change for rendering
		activeImageData->changePlaneInput = true;

		//data infos
		statusLabel->setText("Last loaded file: " + activeImageData->fileName);
		this->ui->label_file->setText(activeImageData->fileName);

		//set close button
		this->ui->pushButton_close->setEnabled(!documentModel.empty());
	}else{
		QMessageBox::information(this,tr("ERROR"), tr("No valid Data selected"));
	}
}
void OCTview3R::slotExit()
{
	qApp->exit();
}
void OCTview3R::slotShowObject(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->showObject = value;
	refreshViewer();
		if(ui->actionObject->isChecked() != value)
			ui->actionObject->setChecked(value);
		if(ui->groupBox_object->isChecked() != value)
			ui->groupBox_object->setChecked(value);
	}
}
void OCTview3R::slotShowPlane(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded && activeImageData->isVolume){
		activeImageData->changePlaneInput = true;
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
	self->refreshViewer();
		self->onePlaneCallbackMutex = true;
	}
}
void OCTview3R::slotCheckFlipPlane(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->switchPlane = value;
	refreshViewer();
	}
}

void OCTview3R::slotSetThreshold()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->currentMinThreshold = this->ui->Slider_minThreshold->value();
		activeImageData->currentMaxThreshold = this->ui->Slider_maxThreshold->value();
		//TODO: move threshold to ImageData?
		activeImageData->threshold->ThresholdBetween((double)activeImageData->currentMinThreshold, (double)activeImageData->currentMaxThreshold);
		activeImageData->threshold->ReplaceOutOn();
		activeImageData->threshold->SetOutValue(0);
		activeImageData->threshold->Update();
	refreshViewer();
	}
}
void OCTview3R::slotCheckMinThresholdSlider(int value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		if(value >= activeImageData->currentMaxThreshold){
			this->ui->Slider_minThreshold->setValue(activeImageData->currentMaxThreshold);
		}
	}
}
void OCTview3R::slotCheckMaxThresholdSlider(int value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
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
void OCTview3R::slotDataFileDialogClosed(vtkImageData* tmpData)
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
		data->VOI[0]		= static_cast<double>(extent[0]);
		data->VOI[1]		= static_cast<double>(extent[1] + 1);
		data->VOI[2]		= static_cast<double>(extent[2]);
		data->VOI[3]		= static_cast<double>(extent[3] + 1);
		data->VOI[4]		= static_cast<double>(extent[4]);
		data->VOI[5]		= static_cast<double>(extent[5] + 1);
		for (int i = 0; i < 6; ++i)
			data->sourceVOI[i] = data->VOI[i];
		tmpData->GetSpacing(data->spacing);
		data->pointSize		= 1;
		data->fileLoaded	= true;
		data->poly			= nullptr;
		data->image			= tmpData;
		tmpData->Delete();

		// Transfer ownership to the document model and add a matching tab.
		settings.numMaxIndex++;
		const int newIndex = documentModel.activeIndex();
		if(hadLoadedData){
			QWidget* newTab = new QWidget();
			newTab->setObjectName(QString("tab"+QString().setNum(settings.numMaxIndex)));
			this->ui->tabWidget->addTab(newTab, data->typeName + " " + QString().setNum(settings.numMaxIndex));
		}else if(this->ui->tabWidget->count() == 0){
			this->ui->tabWidget->addTab(new QWidget(), data->typeName + " " + QString().setNum(settings.numMaxIndex));
		}
		this->ui->tabWidget->setTabText(newIndex, data->typeName + " " + QString().setNum(settings.numMaxIndex));
		this->ui->tabWidget->setCurrentIndex(newIndex);
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
		tmpPoly->Delete();
		
		// Transfer ownership to the document model and add a matching tab.
		settings.numMaxIndex++;
		const int newIndex = documentModel.activeIndex();
		if(hadLoadedData){
			QWidget* newTab = new QWidget();
			newTab->setObjectName(QString("tab"+QString().setNum(settings.numMaxIndex)));
			this->ui->tabWidget->addTab(newTab, data->typeName + " " + QString().setNum(settings.numMaxIndex));
		}else if(this->ui->tabWidget->count() == 0){
			this->ui->tabWidget->addTab(new QWidget(), data->typeName + " " + QString().setNum(settings.numMaxIndex));
		}
		this->ui->tabWidget->setTabText(newIndex, data->typeName + " " + QString().setNum(settings.numMaxIndex));
		this->ui->tabWidget->setCurrentIndex(newIndex);
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
		openData->setEnabled(false);
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
		openPoly->setEnabled(false);
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
	openData->setEnabled(true);
	QMessageBox::critical(this, tr("Volume loading failed"), message);
}

void OCTview3R::slotPolyLoadFailed(const QString& message)
{
	openPoly->setEnabled(true);
	QMessageBox::critical(this, tr("Polygonal-data loading failed"), message);
}

void OCTview3R::refreshViewer()
{
	if (!viewerController)
		return;

	viewerController->refresh(documentModel, activeImageData, settings);
	if (settings.firstFileLoaded)
	{
		settings.firstFileLoaded = false;
		slotFrontZ();
	}
}

void OCTview3R::slotSetColormap(QString value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->colormapName = value;
		refreshViewer();
	}
}
void OCTview3R::slotPickVolumeColor()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		QColor color = QColorDialog::getColor(activeImageData->volumeColor, this);
		if(color.isValid()){
			activeImageData->volumeColor = color;
			this->ui->pushButton_pickVolumeColor->setStyleSheet("background-color: "+activeImageData->volumeColor.name());
	refreshViewer();
		}
	}
}
void OCTview3R::slotPickPolyColor()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		QColor color = QColorDialog::getColor(activeImageData->polyColor, this);
		if(color.isValid()){
			activeImageData->polyColor = color;
			this->ui->pushButton_pickPolyColor->setStyleSheet("background-color: "+activeImageData->polyColor.name());
	refreshViewer();
		}
	}
}
void OCTview3R::slotShowScalarBar(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
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
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
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
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
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
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
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
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->adjustColormap = value;
		slotSetThreshold();
	refreshViewer();
	}
}
void OCTview3R::slotInvertColormap(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->invertColormap = value;
		slotSetThreshold();
	refreshViewer();
	}
}
void OCTview3R::slotMedianCheckBox(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->checkMedian = value;
		if(value){ 
			activeImageData->median->SetKernelSize(activeImageData->medianKernelX,activeImageData->medianKernelY,activeImageData->medianKernelZ);
		}else{
			activeImageData->median->SetKernelSize(1,1,1);
		}
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
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->changePlaneInput = true;
		activeImageData->orientChanged = true;
		activeImageData->orientIndex = value;
	refreshViewer();
	}
}
void OCTview3R::slotGetPlaneData()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
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
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->rot[0] = value;
	refreshViewer();
	}
}
void OCTview3R::slotRotY(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->rot[1] = value;
	refreshViewer();
	}
}
void OCTview3R::slotRotZ(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->rot[2] = value;
	refreshViewer();
	}
}
void OCTview3R::slotShiftX(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->shift[0] = value;
	refreshViewer();
	}
}
void OCTview3R::slotShiftY(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->shift[1] = value;
	refreshViewer();
	}
}
void OCTview3R::slotShiftZ(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->shift[2] = value;
	refreshViewer();
	}
}
void OCTview3R::slotApplyRanges()
{
	if(!settings.oneFileLoaded || activeImageData == nullptr ||
	   !activeImageData->fileLoaded || !activeImageData->isVolume)
		return;

	const double requestedVOI[6] = {
		ui->x0DoubleSpinBox->value(), ui->x1DoubleSpinBox->value(),
		ui->y0DoubleSpinBox->value(), ui->y1DoubleSpinBox->value(),
		ui->z0DoubleSpinBox->value(), ui->z1DoubleSpinBox->value()
	};
	const bool ordered =
		requestedVOI[0] < requestedVOI[1] &&
		requestedVOI[2] < requestedVOI[3] &&
		requestedVOI[4] < requestedVOI[5];
	bool insideSource = true;
	for (int i = 0; i < 6; i += 2)
	{
		insideSource = insideSource &&
			requestedVOI[i] >= activeImageData->sourceVOI[i] &&
			requestedVOI[i + 1] <= activeImageData->sourceVOI[i + 1];
	}
	if (!ordered || !insideSource)
	{
		QMessageBox::information(
			this,
			tr("Invalid ranges"),
			tr("Each range start must be smaller than its end and remain inside the source extent."));
		return;
	}

	for (int i = 0; i < 6; ++i)
		activeImageData->VOI[i] = requestedVOI[i];
	activeImageData->changePlaneInput = true;
	activeImageData->orientChanged = true;
	refreshViewer();
}
void OCTview3R::slotPlaneUp()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
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
			refreshViewer();
		}else{
			QMessageBox::information(this,tr("WARNING"), tr("Plane not orthogonal to x, y or z."));	
		}
		slotGetPlaneData();
	}
}
void OCTview3R::slotPlaneDown()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
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
			refreshViewer();
		}else{
			QMessageBox::information(this,tr("WARNING"), tr("Plane not orthogonal to x, y or z."));	
		}
		slotGetPlaneData();
	}
}

void OCTview3R::slotSetPointSize(int value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->pointSize = value;
	refreshViewer();
	}
}

void OCTview3R::slotSetObjectOpacity(int value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->objectOpacity = (double)value * 0.01;
	refreshViewer();
	}
}

void OCTview3R::slotSetBlendMode(int index)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->blendMode = index;
	refreshViewer();
	}
}
void OCTview3R::slotSetPolyMode(int index)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->polyMode = index;
	refreshViewer();
	}
}
void OCTview3R::slotPlaneVisibility(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->planeIsVisible = value;
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
void OCTview3R::slotScaleX(double value)
{		
	settings.x_fac = value;
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		viewerController->refreshAll(documentModel, activeImageData, settings);
	}
}
void OCTview3R::slotScaleY(double value)
{
	settings.y_fac = value;
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		viewerController->refreshAll(documentModel, activeImageData, settings);
	}
}
void OCTview3R::slotScaleZ(double value)
{
	settings.z_fac = value;
	if(settings.oneFileLoaded && activeImageData != nullptr && activeImageData->fileLoaded){
		viewerController->refreshAll(documentModel, activeImageData, settings);
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
	const int index = ui->tabWidget->currentIndex();
	if (index < 0 || index >= documentModel.size())
		return;

	DocumentModel::Document data = documentModel.takeAt(index);
	viewerController->detach(*data);

	QWidget* page = ui->tabWidget->widget(index);
	ui->tabWidget->removeTab(index);
	if (page)
		page->deleteLater();
	settings.oneFileLoaded = !documentModel.empty();
	activeImageData = documentModel.active();

	if (documentModel.empty())
	{
		ui->tabWidget->addTab(new QWidget(), tr("Open File"));
		ui->pushButton_close->setEnabled(false);
		ui->pushButton_render->setEnabled(false);
		ui->groupBox_object->setEnabled(false);
		ui->groupBox_plane->setEnabled(false);
		ui->groupBox_threshold->setEnabled(false);
		ui->groupBox_colorMapping->setEnabled(false);
		ui->actionObject->setCheckable(false);
		ui->actionPlane->setCheckable(false);
		ui->label_file->setText(tr("File:"));
		statusLabel->setText(tr("No file loaded."));
		viewerController->refresh(documentModel, nullptr, settings);
		return;
	}

	const int nextIndex = qMin(index, documentModel.size() - 1);
	ui->tabWidget->setCurrentIndex(nextIndex);
	slotSetImageData(nextIndex);
}

#if 0
// Legacy experimental visualization routines retained for reference only.
// They depend on machine-specific paths and are intentionally excluded from
// the production build.
void OCTview3R::magic()
{

// Create a grid
  vtkSmartPointer<vtkStructuredGridReader> reader = vtkSmartPointer<vtkStructuredGridReader>::New();
  std::string name = "ImageData/example.vtk";
  reader->SetFileName(name.c_str());

  vtkSmartPointer<vtkStructuredGridGeometryFilter>	gridfilter = vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
  gridfilter->SetInputConnection(reader->GetOutputPort());

  // Create a mapper and actor
  vtkSmartPointer<vtkDataSetMapper> mapper = vtkSmartPointer<vtkDataSetMapper>::New();
  //mapper->SetInputData(structuredGrid);
  mapper->SetInputConnection(gridfilter->GetOutputPort());
 
  vtkSmartPointer<vtkActor> actor =
    vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(mapper);
 
  // Create a renderer, render window, and interactor
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindow->AddRenderer(renderer);
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractor->SetRenderWindow(renderWindow);
 
  // Add the actor to the scene
  renderer->AddActor(actor);
  renderer->SetBackground(.3, .6, .3); // Background color green
 
  // Render and interact
  renderWindow->Render();
  renderWindowInteractor->Start();
 
}

/*
#include <iostream>
#include <fstream>

// Thanks to https://stackoverflow.com/questions/105252
template <typename T>
void SwapEnd(T& var)
{
  char* varArray = reinterpret_cast<char*>(&var);
  for(long i = 0; i < static_cast<long>(sizeof(var)/2); i++)
    std::swap(varArray[sizeof(var) - 1 - i],varArray[i]);
}

double myarray[72] = {
  0.001,0.002,0,1,0,0,2,0,0,3,0,0,4,0,0,
  5,0,0,0,1,0,1,1,0,2,1,0,3,1,0,
  4,1,0,5,1,0,0,2,0,1,2,0,2,2,0,
  3,2,0,4,2,0,5,2,0,0,3,0,1,3,0,
  2,3,0,3,3,0,4,3,0,5,3,0};

int main()
{
  std::ofstream vtkstream;
  vtkstream.open("test.vtk", std::ios::out | std::ios::app | std::ios::binary);
  if (vtkstream) {
    vtkstream<<"# vtk DataFile Version 2.0"<<"\n";
    vtkstream<<"Exemple"<<"\n";
    vtkstream<<"BINARY"<<"\n";
    vtkstream<<"DATASET STRUCTURED_GRID"<<std::endl;
    vtkstream<<"DIMENSIONS 6 4 1"<<std::endl;
    vtkstream<<"POINTS 24 double"<<std::endl;
    for (unsigned int i = 0; i < 72; ++i) {
      SwapEnd(myarray[i]);
      vtkstream.write((char*)&myarray[i], sizeof(double));
    }
    vtkstream.close();
  } else {
    std::cout<<"ERROR"<<std::endl;
  }
  return 0;
}
*/

void OCTview3R::transform()
{
	QString path = "ImageData/";
	//QString path = "ImageData/";
	
	/*
	QString fileName = "";
	QFileDialog getFileDialog(this, "Open File", path, "VTK Image Data (*.vtk *.tif)");
	getFileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	QStringList filters;
	filters << "VTK files (*.vtk)" << "TIF files (*.tif)";
	getFileDialog.setNameFilters(filters);
	if(getFileDialog.exec() == QDialog::Accepted){
		QString selectedFilter = getFileDialog.selectedNameFilter();
		fileName = getFileDialog.selectedFiles()[0];
	}else{
		QMessageBox::information(this,tr("WARNING"), tr("No valid image data selected"));	
	}
	*/

	vtkSmartPointer<vtkActor> actor1 = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> actor2 = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> actor3 = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> actor4_1 = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> actor4_2 = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> actor4_3 = vtkSmartPointer<vtkActor>::New();

	//1 - geht mit segmentierung
	vtkSmartPointer<vtkPolyDataReader>			reader1 = vtkSmartPointer<vtkPolyDataReader>::New();
	vtkSmartPointer<vtkPolyDataMapper>			mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	//2 - geht mit segmentierung
	vtkSmartPointer<vtkGenericDataObjectReader> reader2 = vtkSmartPointer<vtkGenericDataObjectReader>::New();
	vtkSmartPointer<vtkDataSetMapper>			mapper2_1 = vtkSmartPointer<vtkDataSetMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper>			mapper2_2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	//3 - vtk image reader
	vtkSmartPointer<vtkStructuredPointsReader>	reader3 = vtkSmartPointer<vtkStructuredPointsReader>::New();
	vtkSmartPointer<vtkSmartVolumeMapper>		mapper3	= vtkSmartPointer<vtkSmartVolumeMapper>::New();
	vtkSmartPointer<vtkVolume>					volume3 = vtkSmartPointer<vtkVolume>::New();
	vtkSmartPointer<vtkVolumeProperty>			proper3 = vtkSmartPointer<vtkVolumeProperty>::New();
	//4 - tiff image
	vtkSmartPointer<vtkTIFFReader>				reader4 = vtkSmartPointer<vtkTIFFReader>::New();
	vtkSmartPointer<vtkSmartVolumeMapper>		mapper4_1	= vtkSmartPointer<vtkSmartVolumeMapper>::New();
	vtkSmartPointer<vtkSmartVolumeMapper>		mapper4_2	= vtkSmartPointer<vtkSmartVolumeMapper>::New();
	vtkSmartPointer<vtkImageReslice>			reslice4 = vtkSmartPointer<vtkImageReslice>::New();
	vtkSmartPointer<vtkImageResliceMapper>		resmap4 = vtkSmartPointer<vtkImageResliceMapper>::New();
	vtkSmartPointer<vtkVolume>					volume4 = vtkSmartPointer<vtkVolume>::New();
	vtkSmartPointer<vtkImageData>				data4 = vtkSmartPointer<vtkImageData>::New();
	vtkSmartPointer<vtkStructuredGrid>			grid4 = vtkSmartPointer<vtkStructuredGrid>::New();
	vtkSmartPointer<vtkVolumeProperty>			proper4 = vtkSmartPointer<vtkVolumeProperty>::New();
	vtkSmartPointer<vtkImageDataGeometryFilter> filter4 = vtkSmartPointer<vtkImageDataGeometryFilter>::New();
	vtkSmartPointer<vtkPolyDataMapper>			polymapper4_1 = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper>			polymapper4_2 = vtkSmartPointer<vtkPolyDataMapper>::New();
	//5 - structured grid
	vtkSmartPointer<vtkStructuredGridReader>			gridreader = vtkSmartPointer<vtkStructuredGridReader>::New();
	vtkSmartPointer<vtkStructuredGrid>					grid = vtkSmartPointer<vtkStructuredGrid>::New();
	vtkSmartPointer<vtkStructuredGridGeometryFilter>	gridfilter = vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
	vtkSmartPointer<vtkPolyDataMapper>					gridmapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor>							gridactor = vtkSmartPointer<vtkActor>::New();
	//6 - structured points
	vtkSmartPointer<vtkStructuredPointsReader>			pointsreader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	vtkSmartPointer<vtkStructuredGrid>					points = vtkSmartPointer<vtkStructuredGrid>::New();
	vtkSmartPointer<vtkStructuredGridGeometryFilter>	pointsfilter = vtkSmartPointer<vtkStructuredGridGeometryFilter>::New();
	vtkSmartPointer<vtkPolyDataMapper>					pointsmapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor>							pointsactor = vtkSmartPointer<vtkActor>::New();

	vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
	vtkSmartPointer<vtkTransformPolyDataFilter> transformPDFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	vtkSmartPointer<vtkTransformFilter> transformFilter = vtkSmartPointer<vtkTransformFilter>::New();
	vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
	
	QString fileName1 = path + "100x100x100_16bit_ITKSnap.vtk";
	QString fileName2 = path + "/100x100x100_8bit_Mesh_Zelle_vonRAW_noMinus.vtk";
	QString fileName3 = path + "/100x100x100_8bit_JUST_POINTS.vtr";
	QString fileName5 = path + "/struct.vtr";
	

	// tiff to actor
	QString fileName4 = path + "100x100x100_8bit.tif";
	reader4->SetFileName(fileName4.toLocal8Bit());
	reader4->Update();

	filter4->SetInputConnection(reader4->GetOutputPort());
	polymapper4_1->SetInputConnection(filter4->GetOutputPort());
	actor4_1->SetMapper(polymapper4_1);
	
		if(true){
			double *center;
			center = actor4_1->GetCenter();
			double c[3];
			c[0] = *center;
			c[1] = *(++center);
			c[2] = *(++center);
		
			if((c[0]<0) || (c[1]<0))
				transform->RotateZ(180);
		
			double bounds[6];
			actor4_1->GetBounds(bounds);
			double b[3];
			b[0] = 0.5*(bounds[0]+bounds[1]);
			b[1] = 0.5*(bounds[2]+bounds[3]);
			b[2] = 0.5*(bounds[4]+bounds[5]);

					// Apply the transforms
					double startPoint[3] = {c[0],c[1],c[2]};
					double endPoint[3]   = {-c[0],-c[1],-c[2]};

					transform->Translate(startPoint);
						double scale[3];
						scale[0] = 4.0;
						scale[1] = 1.0;
						scale[2] = 1.0;
						transform->Scale(scale[0], scale[1], scale[2]);
						double rot[3];
						rot[0] = 45.0;
						rot[1] = 0.0;
						rot[2] = 0.0;
						transform->RotateX(rot[0]);
						transform->RotateY(rot[1]);
						transform->RotateZ(rot[2]);
					transform->Translate(endPoint);

					double shift[3];
					shift[0] = 60.0;
					shift[1] = 0.0;
					shift[2] = 0.0;
					transform->Translate(shift);

					// Transform data
					transformFilter->SetTransform(transform);						
					transformFilter->SetInputData(reader4->GetOutput());
					//grid4 = (vtkStructuredGrid*) transformFilter->GetOutput();

					// Transform the polydata
					transformPDFilter->SetTransform(transform);						
					transformPDFilter->SetInputConnection(filter4->GetOutputPort());
		
			polymapper4_2->SetInputConnection(transformPDFilter->GetOutputPort());
			actor4_2->SetMapper(polymapper4_2);
			double position[3];
			actor4_2->GetPosition(position);
			//actor4_2->GetProperty()->SetRepresentationToPoints();
			//actor4_2->GetProperty()->SetRepresentationToWireframe();
			//actor4_2->GetProperty()->SetOpacity(0.5);
			//actor4_2->GetProperty()->SetRepresentationToSurface();
			//actor4_2->GetProperty()->SetColor(0.0,1.0,0.0);
		}

		mapper4_1->SetInputConnection(reader4->GetOutputPort());
		mapper4_1->SetBlendModeToMaximumIntensity();
		//mapper4_1->SetBlendModeToComposite();
		volume4->SetMapper(mapper4_1);
		//volume4->SetScale(4.0,4.0,1.0);
/*
		reader2->SetFileName(fileName2.toLocal8Bit());
		reader2->Update();
		//mapper2->SetInputConnection(geometryFilter->GetOutputPort());
		mapper2->SetInputConnection(reader2->GetOutputPort());
		//mapper2->ScalarVisibilityOff();
		actor2->SetMapper(mapper2);
		//actor2->GetProperty()->SetRepresentationToPoints();
		//actor2->GetProperty()->SetRepresentationToWireframe();
		actor2->GetProperty()->SetOpacity(0.2);
		actor2->GetProperty()->SetRepresentationToSurface();
		actor2->GetProperty()->SetColor(1.0,0.0,0.0);

		// load tiff data
		QString tiffName;
		tiffName = path + "/100x100x100_8bit.tif";
		//tiffName = "ImageData/100x100x100_8bit.tif";
		reader4->SetFileName(tiffName.toLocal8Bit());
		reader4->Update();
		mapper4->SetInputConnection(reader4->GetOutputPort());
		mapper4->SetBlendModeToMaximumIntensity();
		//mapper4->SetBlendModeToComposite();
		volume4->SetMapper(mapper4);
		//volume4->SetVisibility(true);	
		//volume4->Update();

		// load vtk image data
		QString vtkName;
		vtkName = path + "/100x100x100_8bit.vtk";
		reader3->SetFileName(vtkName.toLocal8Bit());
		reader3->Update();
		mapper3->SetInputConnection(reader3->GetOutputPort());
		mapper3->SetBlendModeToMaximumIntensity();
		//mapper3->SetBlendModeToComposite();
		volume3->SetMapper(mapper3);
*/
	vtkSmartPointer<vtkRenderer> renderer4 = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer4);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);
	renderWindowInteractor->Initialize();

    vtkSmartPointer<vtkCubeAxesActor> axes4 =  vtkSmartPointer<vtkCubeAxesActor>::New();
	axes4->SetXAxisLabelVisibility(1);
	axes4->SetYAxisLabelVisibility(1);
	axes4->SetZAxisLabelVisibility(1);
	axes4->SetXAxisTickVisibility(1);
	axes4->SetYAxisTickVisibility(1);
	axes4->SetZAxisTickVisibility(1);
	axes4->SetScreenSize(12.0);
	axes4->SetFlyModeToOuterEdges();
	axes4->SetCornerOffset(0.0);
	axes4->SetBounds(volume4->GetBounds());
	axes4->SetCamera(renderer4->GetActiveCamera());
	renderer4->AddViewProp(axes4);


	renderer4->ResetCamera();
	renderer4->SetBackground(0.1,0.2,0.4);
	renderer4->AddActor(actor4_2);

	//renderer4->AddActor(actor4_3);
	renderer4->AddVolume(volume4);
	renderWindow->Render();
	renderWindowInteractor->Start();

	/*
	// load vts structured grid
	gridreader->SetFileName(fileName4.toLocal8Bit());
	gridreader->Update();
	gridfilter->SetInputData(gridreader->GetOutput());
	gridfilter->Update();
	gridmapper->SetInputConnection(gridfilter->GetOutputPort());
	gridactor->SetMapper(gridmapper);
	gridactor->GetProperty()->SetColor(1.0,0.0,0.0);
	*/

	/*
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->ShadeOff();
	volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
	vtkSmartPointer<vtkPiecewiseFunction> compositeOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	compositeOpacity->AddPoint(0.0,0.0);
	compositeOpacity->AddPoint(80.0,1.0);
	compositeOpacity->AddPoint(80.1,0.0);
	compositeOpacity->AddPoint(255.0,0.0);
	volumeProperty->SetScalarOpacity(compositeOpacity); // composite first.
	volume4->SetProperty(volumeProperty);
	*/
	
/*
  vtkSmartPointer<vtkPolyDataReader> readerT = vtkSmartPointer<vtkPolyDataReader>::New();
  readerT->SetFileName(fileName2.toLocal8Bit());
  readerT->Update();

  vtkSmartPointer<vtkPolyData> inputPolyData = vtkSmartPointer<vtkPolyData>::New();
  //inputPolyData->CopyStructure(readerT->GetOutput());
  inputPolyData = gridfilter->GetOutput();

  // Triangulate the grid points
  vtkSmartPointer<vtkDelaunay2D> delaunay = vtkSmartPointer<vtkDelaunay2D>::New();
  delaunay->SetInputData(inputPolyData);
  delaunay->Update();

  vtkSmartPointer<vtkPolyDataMapper> meshMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  meshMapper->SetInputConnection(delaunay->GetOutputPort());

  vtkSmartPointer<vtkActor> meshActor = vtkSmartPointer<vtkActor>::New();
  meshActor->SetMapper(meshMapper);
  meshActor->GetProperty()->SetInterpolationToFlat();
  meshActor->GetProperty()->SetRepresentationToWireframe();

  vtkSmartPointer<vtkVertexGlyphFilter> glyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
  glyphFilter->SetInputData(inputPolyData);
  glyphFilter->Update();

  // warp plane
  vtkSmartPointer<vtkWarpScalar> warpT = vtkSmartPointer<vtkWarpScalar>::New();
  warpT->SetInputData(inputPolyData);
  warpT->SetScaleFactor(0.0);

  // Visualize
  vtkSmartPointer<vtkDataSetMapper> mapperT = vtkSmartPointer<vtkDataSetMapper>::New();
  mapperT->SetInputConnection(warpT->GetOutputPort());
  mapperT->SetScalarRange(0,1000);

  vtkSmartPointer<vtkLookupTable> lookupTableT = vtkSmartPointer<vtkLookupTable>::New();
  lookupTableT->SetTableRange(0.0, 1.0);
  // If you don't want to use the whole color range, you can use
  // SetValueRange, SetHueRange, and SetSaturationRange
  lookupTable->Build();
  mapperT->SetLookupTable(lookupTable);

  vtkSmartPointer<vtkActor> actorT = vtkSmartPointer<vtkActor>::New();
  actorT->GetProperty()->SetPointSize(4);
  actorT->SetMapper(mapperT);

  vtkSmartPointer<vtkRenderer> rendererT = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renderWindowT = vtkSmartPointer<vtkRenderWindow>::New();
  renderWindowT->AddRenderer(rendererT);
  vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractorT = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renderWindowInteractorT->SetRenderWindow(renderWindowT);

  rendererT->AddActor(actorT);
  rendererT->AddActor(meshActor);
  rendererT->SetBackground(.1, .1, .3);
  renderWindowT->Render();

  vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  renderWindowInteractorT->SetInteractorStyle(style);

  // add & render CubeAxes
  vtkSmartPointer<vtkCubeAxesActor2D> axes = vtkSmartPointer<vtkCubeAxesActor2D>::New();
  axes->SetInputData(warpT->GetOutput());
  axes->SetFontFactor(3.0);
  axes->SetFlyModeToNone();
  axes->SetCamera(rendererT->GetActiveCamera());
  
  vtkSmartPointer<vtkAxisActor2D> xAxis = axes->GetXAxisActor2D();
  xAxis->SetAdjustLabels(1);
  rendererT->AddViewProp(axes);
  renderWindowInteractorT->Start();
  */
}
#endif
