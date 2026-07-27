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
#include <QMessageBox>
#include <QTimer>
#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QProgressDialog>
#include <qfileinfo.h>

OCTview3R::OCTview3R()
{
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

	moveCameraCallbackMutex			= false;
	onePlaneCallbackMutex			= false;

	//general
	renderer					= vtkRenderer::New();
	renWin						= vtkRenderWindow::New();
	iren						= vtkRenderWindowInteractor::New();
	scalarBarActor				= vtkScalarBarActor::New();
	scalarBarWidget				= vtkScalarBarWidget::New();
	axes						= vtkCubeAxesActor::New();
	lookupTable					= vtkLookupTable::New();
	im							= vtkImageResliceMapper::New();
	ip							= vtkImageProperty::New();
	ia							= vtkImageSlice::New();
	cam							= vtkCamera::New();
	camTrans					= vtkTransform::New();
	math						= vtkMath::New();
	axesActor					= vtkAxesActor::New();
	orientWidget				= vtkOrientationMarkerWidget::New();
	//rayCastCompositeFunction	= vtkVolumeRayCastCompositeFunction::New();
	//rayCastMIPFunction			= vtkVolumeRayCastMIPFunction::New();
	//volumeRayCastMapper			= vtkVolumeRayCastMapper::New();

	//setup ui pointer
	this->ui = new Ui_OCTview3R;
	this->ui->setupUi(this);
	this->showMaximized();
	delete this->ui->centralwidget;

	//POLY TESTING
	connect(this->ui->pushButton_magic, SIGNAL(clicked()), this, SLOT(transform()));

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
	connect(this->ui->planeOrientationComboBox, SIGNAL(highlighted(int)), this, SLOT(slotOrientationChanged(int)));
	connect(this->ui->planePushButton, SIGNAL(clicked()), this, SLOT(slotGetPlaneData()));
	connect(this->ui->pushButton_render, SIGNAL(clicked()), this, SLOT(slotRenderAgain()));
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
	connect(this->ui->x0DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotX0(double)));
	connect(this->ui->x1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotX1(double)));
	connect(this->ui->y0DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotY0(double)));
	connect(this->ui->y1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotY1(double)));
	connect(this->ui->z0DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotZ0(double)));
	connect(this->ui->z1DoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotZ1(double)));
	connect(this->ui->rotYDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotRotY(double)));
	connect(this->ui->rotZDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(slotRotZ(double)));
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

	openData = new OpenData(0);
	connect(openData, SIGNAL(signalStartProcess()), this, SLOT(slotProcessDataFile()));
	openPoly = new OpenPoly(0);
	connect(openPoly, SIGNAL(signalStartProcess()), this, SLOT(slotProcessPolyFile()));

	//setup statusBar
	statusLabel = new QLabel();
	this->ui->statusBar->addWidget(statusLabel);
	statusLabel->setText("No file loaded.");

	initializeVTKPipeline();
}

OCTview3R::~OCTview3R()
{

}

ImageData *OCTview3R::getInitializedImageData()
{
	ImageData *imageData = new ImageData();

	//new file
	imageData->fileChanged			= true;

	//vtk
	imageData->threshold			= vtkSmartPointer<vtkImageThreshold>::New();
	imageData->colorFun				= vtkSmartPointer<vtkColorTransferFunction>::New();
	imageData->opacityFun			= vtkSmartPointer<vtkPiecewiseFunction>::New();
	imageData->planeWidget			= vtkSmartPointer<vtkImagePlaneWidget>::New();
	imageData->median				= vtkSmartPointer<vtkImageMedian3D>::New();
	imageData->extractVOI			= vtkSmartPointer<vtkExtractVOI>::New();
	imageData->volume				= vtkSmartPointer<vtkVolume>::New();
	imageData->actor				= vtkSmartPointer<vtkActor>::New();
	imageData->transform			= vtkSmartPointer<vtkTransform>::New();
	imageData->luminance			= vtkSmartPointer<vtkImageLuminance>::New();
	imageData->colorMap				= vtkSmartPointer<vtkImageMapToColors>::New();
	imageData->volumeMapper			= vtkSmartPointer<vtkSmartVolumeMapper>::New();
	imageData->polyMapper			= vtkSmartPointer<vtkPolyDataMapper>::New();
	imageData->polyActor			= vtkSmartPointer<vtkActor>::New();
	imageData->clipPlane			= vtkSmartPointer<vtkPlane>::New();
	imageData->implicitPlane		= vtkSmartPointer<vtkImplicitPlaneWidget>::New();
	imageData->image				= vtkSmartPointer<vtkImageData>::New();
	imageData->object				= vtkSmartPointer<vtkAlgorithm>::New();
	imageData->output				= vtkSmartPointer<vtkAlgorithmOutput>::New();
	imageData->planeCollection		= vtkSmartPointer<vtkPlaneCollection>::New();

	//data
	imageData->bitsize				= bitsizeType(-1);
	imageData->endian				= endianType(-1);
	imageData->polyFormat			= polyType(-1);
	imageData->dataFormat			= dataType(-1);
	imageData->width				= -1;
	imageData->height				= -1;
	imageData->depth				= -1;

	//object
	imageData->fileName				= "";
	imageData->filePath				= "";
	imageData->isPolyData			= false;
	imageData->isVolume				= false;
	imageData->VOI[0]				= 0.0;
	imageData->VOI[1]				= 0.0;
	imageData->VOI[2]				= 0.0;
	imageData->VOI[3]				= 0.0;
	imageData->VOI[4]				= 0.0;
	imageData->VOI[5]				= 0.0;
	imageData->rot[0]				= 0.0;
	imageData->rot[1]				= 0.0;
	imageData->rot[2]				= 0.0;
	imageData->shift[0]				= 0.0;
	imageData->shift[1]				= 0.0;
	imageData->shift[2]				= 0.0;
	imageData->spacing[0]			= 1.0;
	imageData->spacing[1]			= 1.0;
	imageData->spacing[2]			= 1.0;
	imageData->pointSize			= 1.0;
	imageData->showObject			= false;
	imageData->objectOpacity		= 0.5;

	//color
	imageData->colormapName			= "Greyscale";
	imageData->adjustColormap		= true;
	imageData->invertColormap		= false;
	imageData->blendMode			= 0;
	imageData->polyMode				= 0;
	imageData->polyColor.setRgb(255,0,0);
	imageData->volumeColor.setRgb(255,0,0);

	//threshold
	imageData->changedThreshold		= false;
	imageData->minValue				= 0;
	imageData->maxValue				= 255;
	imageData->currentMinThreshold	= 0;
	imageData->currentMaxThreshold	= 255;

	//plane
	imageData->initPlane			= true;
	imageData->showPlane			= false;
	imageData->planeOrigin[0]		= 0.0; 
	imageData->planeOrigin[1]		= 0.0; 
	imageData->planeOrigin[2]		= 0.0;
	imageData->checkMedian			= false;
	imageData->medianKernelX		= 1;
	imageData->medianKernelY		= 1;
	imageData->medianKernelZ		= 1;
	imageData->orientIndex			= 0;
	imageData->orientChanged		= false;
	imageData->changePlaneInput		= false;
	imageData->switchPlane			= false;
	imageData->planeIsVisible		= true;

	return imageData;
}

void OCTview3R::slotResetCam()
{
	renderer->ResetCamera();
	iren->Render();
}
void OCTview3R::initializeVTKPipeline()
{
	// RENDERER
	renWin = this->ui->qvtkWidget->GetRenderWindow();
	renWin->AddRenderer(renderer);
	iren = renWin->GetInteractor();
	double frameRate = 10.0;
	iren->SetDesiredUpdateRate(frameRate);
	renderer->GradientBackgroundOn();
	renderer->SetBackground(double(settings.background_RGB1[0])/255.0, double(settings.background_RGB1[1])/255.0, double(settings.background_RGB1[2])/255.0);
	renderer->SetBackground2(double(settings.background_RGB2[0])/255.0, double(settings.background_RGB2[1])/255.0, double(settings.background_RGB2[2])/255.0);
	slotResetCam();
}
void OCTview3R::slotSetImageData(int index)
{
	slotSetImageData(imageDataList.at(index));
}

void OCTview3R::slotSetImageData(ImageData* data)
{
	if(data->fileLoaded){
		//set activeImageData to new data
		activeImageData = data;

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
		this->ui->x0DoubleSpinBox->setMinimum(activeImageData->VOI[0]);
		this->ui->x0DoubleSpinBox->setMaximum(activeImageData->VOI[1]);
		this->ui->x0DoubleSpinBox->setValue(activeImageData->VOI[0]);
		this->ui->x1DoubleSpinBox->setMinimum(activeImageData->VOI[0]);
		this->ui->x1DoubleSpinBox->setMaximum(activeImageData->VOI[1]);
		this->ui->x1DoubleSpinBox->setValue(activeImageData->VOI[1]);
		this->ui->y0DoubleSpinBox->setMinimum(activeImageData->VOI[2]);
		this->ui->y0DoubleSpinBox->setMaximum(activeImageData->VOI[3]);
		this->ui->y0DoubleSpinBox->setValue(activeImageData->VOI[2]);
		this->ui->y1DoubleSpinBox->setMinimum(activeImageData->VOI[2]);
		this->ui->y1DoubleSpinBox->setMaximum(activeImageData->VOI[3]);
		this->ui->y1DoubleSpinBox->setValue(activeImageData->VOI[3]);
		this->ui->z0DoubleSpinBox->setMinimum(activeImageData->VOI[4]);
		this->ui->z0DoubleSpinBox->setMaximum(activeImageData->VOI[5]);
		this->ui->z0DoubleSpinBox->setValue(activeImageData->VOI[4]);
		this->ui->z1DoubleSpinBox->setMinimum(activeImageData->VOI[4]);
		this->ui->z1DoubleSpinBox->setMaximum(activeImageData->VOI[5]);
		this->ui->z1DoubleSpinBox->setValue(activeImageData->VOI[5]);
		this->ui->xLabel->setText("["+QString().setNum(activeImageData->VOI[0])+","+QString().setNum(activeImageData->VOI[1])+"]");
		this->ui->yLabel->setText("["+QString().setNum(activeImageData->VOI[2])+","+QString().setNum(activeImageData->VOI[3])+"]");
		this->ui->zLabel->setText("["+QString().setNum(activeImageData->VOI[4])+","+QString().setNum(activeImageData->VOI[5])+"]");
		this->ui->rotXDoubleSpinBox->setValue(activeImageData->rot[0]);
		this->ui->rotYDoubleSpinBox->setValue(activeImageData->rot[1]);
		this->ui->rotZDoubleSpinBox->setValue(activeImageData->rot[2]);
		this->ui->shiftXDoubleSpinBox->setValue(activeImageData->shift[0]);
		this->ui->shiftYDoubleSpinBox->setValue(activeImageData->shift[1]);
		this->ui->shiftZDoubleSpinBox->setValue(activeImageData->shift[2]);
		//mark object change for rendering
		//activeImageData->fileChanged = true;

		//Set colormap values
		this->ui->groupBox_colorMapping->setEnabled(true);
		this->ui->comboBox_blendMode->setCurrentIndex(activeImageData->blendMode);
		this->ui->comboBox_colormapStyle->setCurrentText(activeImageData->colormapName);
		this->ui->comboBox_polyMode->setCurrentIndex(activeImageData->polyMode);
		this->ui->spinBox_pointSize->setMinimum(1);
		this->ui->spinBox_pointSize->setValue(1);
		this->ui->pushButton_pickPolyColor->setStyleSheet("background-color: "+activeImageData->polyColor.name());
		this->ui->pushButton_pickVolumeColor->setStyleSheet("background-color: "+activeImageData->volumeColor.name());
		this->ui->Slider_objectOpacity->setValue(int(100*activeImageData->objectOpacity));
		this->ui->checkBox_adjustColormap->setChecked(activeImageData->adjustColormap);
		this->ui->checkBox_invertColormap->setChecked(activeImageData->invertColormap);

		//Set threshold values
		this->ui->groupBox_threshold->setEnabled(true);		
		this->ui->Slider_minThreshold->setMinimum(0);
		this->ui->Slider_minThreshold->setMaximum(activeImageData->maxValue);
		this->ui->Slider_minThreshold->setValue(activeImageData->currentMinThreshold);
		this->ui->Slider_minThreshold->setEnabled(true);
		this->ui->label_minThreshold->setNum(activeImageData->currentMinThreshold);
		this->ui->Slider_maxThreshold->setMinimum(0);    
		this->ui->Slider_maxThreshold->setMaximum(activeImageData->maxValue);
		this->ui->Slider_maxThreshold->setValue(activeImageData->currentMaxThreshold);
		this->ui->Slider_maxThreshold->setEnabled(true);
		this->ui->label_maxThreshold->setNum(activeImageData->currentMaxThreshold);
		//mark threshold change for rendering
		activeImageData->changedThreshold = true;

		//plane orientation and median kernel
		this->ui->groupBox_plane->setEnabled(true);
		this->ui->actionPlane->setCheckable(true);
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
		this->ui->pushButton_close->setEnabled(this->ui->tabWidget->count()>1);
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
		connectVTKPipeline();
		if(ui->actionObject->isChecked() != value)
			ui->actionObject->setChecked(value);
		if(ui->groupBox_object->isChecked() != value)
			ui->groupBox_object->setChecked(value);
	}
}
void OCTview3R::moveCameraCallbackFunction(vtkObject* caller, unsigned long eventId, void *clientData, void *callData)
{
	OCTview3R *self = reinterpret_cast<OCTview3R*>(clientData);
	if(self->moveCameraCallbackMutex){
		self->moveCameraCallbackMutex = false;
		self->connectVTKPipeline();
		self->getCamRot();
		self->moveCameraCallbackMutex = true;
	}
}
void OCTview3R::slotShowPlane(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->changePlaneInput = true;
		if(value){
			activeImageData->showPlane = true;
			onePlaneCallbackMutex = true;
			vtkSmartPointer<vtkCallbackCommand> startCallback = vtkSmartPointer<vtkCallbackCommand>::New();
			startCallback->SetCallback(onePlaneCallbackFunction);
			startCallback->SetClientData(this);
			activeImageData->planeWidget->AddObserver(vtkCommand::InteractionEvent, startCallback); 
		}else{
			onePlaneCallbackMutex = false;
			activeImageData->showPlane = false;
		}
		connectVTKPipeline();
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
		self->connectVTKPipeline();
		self->onePlaneCallbackMutex = true;
	}
}
void OCTview3R::slotCheckFlipPlane(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->switchPlane = value;
		connectVTKPipeline();
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
		connectVTKPipeline();
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
void OCTview3R::slotDataFileDialogClosed(vtkImageReader2* tmpData)
{
	if(openData->isValidData() && (tmpData != nullptr)){
		ImageData* data		= getInitializedImageData();
		data->showObject	= true;
		data->isVolume		= true;
		data->isPolyData	= false;
		data->typeName		= "Volume";
		data->fileName		= openData->getFileName();
		data->filePath		= openData->getFilePath();
		data->dataFormat	= openData->getDataFormat();
		data->width			= openData->getWidth();
		data->height		= openData->getHeight();
		data->depth			= openData->getDepth();
		data->bitsize		= openData->getBitsize();
		data->endian		= openData->getEndian();
		data->maxValue		= pow(2, 8*((int)data->bitsize+1))-1;
		data->currentMaxThreshold = pow(2, 8*((int)data->bitsize+1))-1;
		data->endian		= openData->getEndian();
		data->VOI[0]		= 0.0;
		data->VOI[1]		= (double)openData->getWidth();
		data->VOI[2]		= 0.0;
		data->VOI[3]		= (double)openData->getHeight();
		data->VOI[4]		= 0.0;
		data->VOI[5]		= (double)openData->getDepth();
		data->spacing[0]	= 1.0;
		data->spacing[1]	= 1.0;
		data->spacing[2]	= 1.0;
		data->pointSize		= 1;
		data->fileLoaded	= true;
		data->poly			= nullptr;
		data->data			= tmpData;

		//allocate new imageData in imageDataList and add tab
		settings.numMaxIndex++;
		imageDataList.append(data);
		int newIndex=imageDataList.indexOf(data);
		if(settings.oneFileLoaded){
			tab = new QWidget();
			tab->setObjectName(QString("tab"+QString().setNum(newIndex)));
			this->ui->tabWidget->addTab(tab, data->typeName + " " + QString().setNum(settings.numMaxIndex));
		}
		this->ui->tabWidget->setTabText(newIndex, data->typeName + " " + QString().setNum(settings.numMaxIndex));
		this->ui->tabWidget->setCurrentIndex(newIndex);
		//set values in GUI
		slotSetImageData(data);
		settings.oneFileLoaded = true;
		//render all
		if(settings.numMaxIndex==1)
			settings.firstFileLoaded = true;
		connectVTKPipeline();
	}else{
		settings.oneFileLoaded = false;
	}
}
void OCTview3R::slotPolyFileDialogClosed(vtkPolyData* tmpPoly)
{
	if(openPoly->isValidData() && (tmpPoly != nullptr)){
		ImageData *data		= getInitializedImageData();
		data->showObject	= true;
		data->isVolume		= false;
		data->isPolyData	= true;
		data->typeName		= "PolyData";
		data->fileName		= openPoly->getFileName();
		data->filePath		= openPoly->getFilePath();
		data->polyFormat	= openPoly->getPolyFormat();
		data->maxValue		= pow(2, 8*((int)data->bitsize+1))-1;
		data->currentMaxThreshold = pow(2, 8*((int)data->bitsize+1))-1;
		std::memcpy(data->VOI, openPoly->getVOI(), 6*sizeof(double));
		data->fileLoaded	= true;
		data->poly			= tmpPoly;
		
		//allocate new imageData in imageDataList and add tab
		imageDataList.append(data);
		int newIndex=imageDataList.indexOf(data);
		if(settings.oneFileLoaded){
			tab = new QWidget();
			tab->setObjectName(QString("tab"+QString().setNum(newIndex)));
			this->ui->tabWidget->addTab(tab, data->typeName + " " + QString().setNum(newIndex+1));
		}
		this->ui->tabWidget->setTabText(newIndex, data->typeName + " " + QString().setNum(newIndex+1));
		this->ui->tabWidget->setCurrentIndex(newIndex);
		settings.numMaxIndex++;
		//set values in GUI
		slotSetImageData(data);
		settings.oneFileLoaded = true;
		//render all
		if(settings.numMaxIndex==1)
			settings.firstFileLoaded = true;
		connectVTKPipeline();
	}else{
		settings.oneFileLoaded = false;
	}
}
void OCTview3R::slotProcessDataFile()
{
	if(openData->isValidData()){
		loading *l = new loading(openData);
		QThread *thread = new QThread(nullptr);
		l->moveToThread(thread);
		connect(l, SIGNAL(updateProgress(int)), openData, SLOT(updateProgress(int)));
		connect(l, SIGNAL(loaded(vtkImageReader2*)), this, SLOT(slotDataFileDialogClosed(vtkImageReader2*)));	//slot with result called after process
		connect(l, SIGNAL(loaded(vtkImageReader2*)), openData, SLOT(doAccepted()));								//close opendata dialog
		connect(l, SIGNAL(loaded(vtkImageReader2*)), thread, SLOT(quit()));										//signal thrown after finishing process
		connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));										//obligatory for clean up
		connect(thread, SIGNAL(started()), l, SLOT(loadData()));												//connecting thread with process
		thread->start();
	}else{
		QMessageBox::information(this,tr("ERROR"), tr("No valid data selected"));
	}
}
void OCTview3R::slotProcessPolyFile()
{
	if(openPoly->isValidData()){
		loading *l = new loading(openPoly);
		QThread *thread = new QThread(nullptr);
		l->moveToThread(thread);
		connect(l, SIGNAL(updateProgress(int)), openPoly, SLOT(updateProgress(int)));
		connect(l, SIGNAL(loaded(vtkPolyData*)), this, SLOT(slotPolyFileDialogClosed(vtkPolyData*)));		//slot with result called after process
		connect(l, SIGNAL(loaded(vtkPolyData*)), openPoly, SLOT(doAccepted()));								//close opendata dialog
		connect(l, SIGNAL(loaded(vtkPolyData*)), thread, SLOT(quit()));										//signal thrown after finishing process
		connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));									//obligatory for clean up
		connect(thread, SIGNAL(started()), l, SLOT(loadPoly()));											//connecting thread with process
		thread->start();
	}else{
		QMessageBox::information(this,tr("ERROR"), tr("No valid data selected"));
	}
}

void OCTview3R::connectVTKPipeline()
{
	for(int i=0; i<settings.numMaxIndex;i++){
		if(imageDataList.at(i)->fileLoaded){
			if(imageDataList.at(i)->isVolume){
				// COLORMAP
				if(imageDataList.at(i)->colormapName=="Greyscale" || imageDataList.at(i)->colormapName==""){
					//Greyscale
					imageDataList.at(i)->colorFun->RemoveAllPoints();
					if(imageDataList.at(i)->adjustColormap){
						if(!imageDataList.at(i)->invertColormap){
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 0.0, 0.0, 0.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 1.0, 1.0, 1.0);
						}else{
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 0.0, 0.0, 0.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 1.0, 1.0, 1.0);
						}
					}else{
						if(!imageDataList.at(i)->invertColormap){
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMinThreshold, 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue, 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue, 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue
							);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMaxThreshold, 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue, 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue, 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue
							);
						}else{
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMaxThreshold, 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue, 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue, 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue
							);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMinThreshold, 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue, 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue, 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue
							);
						}
					}
				}else if(imageDataList.at(i)->colormapName=="Rainbow"){
					//Rainbow
					imageDataList.at(i)->colorFun->RemoveAllPoints();
					imageDataList.at(i)->colorFun->SetColorSpaceToRGB();
					if(!imageDataList.at(i)->invertColormap){
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 0.0, 0.0, 0.5);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.125*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 0.0, 0.0, 1.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.375*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 0.0, 1.0, 1.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.625*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 1.0, 1.0, 0.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.875*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 1.0, 0.0, 0.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 0.5, 0.0, 0.0);
					}else{
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 0.5, 0.0, 0.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.125*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 1.0, 0.0, 0.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.375*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 1.0, 1.0, 0.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.625*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 0.0, 1.0, 1.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.875*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 0.0, 0.0, 1.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 0.0, 0.0, 0.5);
					}
				}else if(imageDataList.at(i)->colormapName=="Black Body"){
					//Dark Body
					imageDataList.at(i)->colorFun->RemoveAllPoints();
					if(imageDataList.at(i)->adjustColormap){
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 0.0, 0.0, 0.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.25*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 1.0, 0.0, 0.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold + 0.75*(imageDataList.at(i)->currentMaxThreshold-imageDataList.at(i)->currentMinThreshold), 1.0, 1.0, 0.0);
						imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 1.0, 1.0, 1.0);
					}else{
						if(imageDataList.at(i)->currentMinThreshold <= 0.25*imageDataList.at(i)->maxValue 
							&& imageDataList.at(i)->currentMaxThreshold <= 0.25*imageDataList.at(i)->maxValue){
								imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 
								4*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue, 0.0, 0.0);
								imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 
								4*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue, 0.0, 0.0);
						}else if(imageDataList.at(i)->currentMinThreshold<=0.25*imageDataList.at(i)->maxValue 
								&& imageDataList.at(i)->currentMaxThreshold >=0.25*imageDataList.at(i)->maxValue 
								&& imageDataList.at(i)->currentMaxThreshold <= 0.75*imageDataList.at(i)->maxValue){
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 
								4*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue, 0.0, 0.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 1.0, 
								(2*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue)-0.5, 0.0);
						}else if(imageDataList.at(i)->currentMinThreshold <= 0.25*imageDataList.at(i)->maxValue 
								&& imageDataList.at(i)->currentMaxThreshold >= 0.75*imageDataList.at(i)->maxValue){
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 
								4*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue, 0.0, 0.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(0.25*imageDataList.at(i)->maxValue, 1.0, 0.0, 0.0 );
							imageDataList.at(i)->colorFun->AddRGBPoint(0.75*imageDataList.at(i)->maxValue, 1.0, 1.0, 0.0 );
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 1.0, 1.0, 
								(4*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue)-3);
						}else if(imageDataList.at(i)->currentMinThreshold > 0.25*imageDataList.at(i)->maxValue 
								&& imageDataList.at(i)->currentMinThreshold <= 0.75*imageDataList.at(i)->maxValue 
								&& imageDataList.at(i)->currentMaxThreshold >=0.25*imageDataList.at(i)->maxValue 
								&& imageDataList.at(i)->currentMaxThreshold <= 0.75*imageDataList.at(i)->maxValue){
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 1.0, 
								(2*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue)-0.5, 0.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 1.0, 
								(2*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue)-0.5, 0.0);
						}else if(imageDataList.at(i)->currentMinThreshold > 0.25*imageDataList.at(i)->maxValue 
								&& imageDataList.at(i)->currentMinThreshold <= 0.75*imageDataList.at(i)->maxValue 
								&& imageDataList.at(i)->currentMaxThreshold >= 0.75*imageDataList.at(i)->maxValue){
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 1.0, 
								(2*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue)-0.5, 0.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(0.75*imageDataList.at(i)->maxValue      , 1.0, 1.0, 0.0 );
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 1.0, 1.0, 
								(4*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue)-3);
						}else{
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMinThreshold, 1.0, 1.0, 
								(4*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue)-3);
							imageDataList.at(i)->colorFun->AddRGBPoint(imageDataList.at(i)->currentMaxThreshold, 1.0, 1.0, 
								(4*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue)-3);
						}
					}
				}
				if(imageDataList.at(i)->colormapName=="White to"){
					//White to
					imageDataList.at(i)->colorFun->RemoveAllPoints();
					if(imageDataList.at(i)->adjustColormap){
						if(!imageDataList.at(i)->invertColormap){
							imageDataList.at(i)->colorFun->AddRGBPoint((double)imageDataList.at(i)->currentMinThreshold, 1.0, 1.0, 1.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMaxThreshold, 
								imageDataList.at(i)->volumeColor.redF(),
								imageDataList.at(i)->volumeColor.greenF(), 
								imageDataList.at(i)->volumeColor.blueF()
							);
						}else{
							imageDataList.at(i)->colorFun->AddRGBPoint((double)imageDataList.at(i)->currentMaxThreshold, 1.0, 1.0, 1.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMinThreshold, 
								imageDataList.at(i)->volumeColor.redF(),
								imageDataList.at(i)->volumeColor.greenF(), 
								imageDataList.at(i)->volumeColor.blueF()
							);
						}
					}else{
						if(!imageDataList.at(i)->invertColormap){
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMinThreshold, 
								1.0+((double)imageDataList.at(i)->volumeColor.redF()-1.0)*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue,
								1.0+((double)imageDataList.at(i)->volumeColor.greenF()-1.0)*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue,
								1.0+((double)imageDataList.at(i)->volumeColor.blueF()-1.0)*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue
							);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMaxThreshold, 
								1.0+((double)imageDataList.at(i)->volumeColor.redF()-1.0)*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue,
								1.0+((double)imageDataList.at(i)->volumeColor.greenF()-1.0)*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue,
								1.0+((double)imageDataList.at(i)->volumeColor.blueF()-1.0)*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue
							);
						}else{
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMaxThreshold, 
								1.0+((double)imageDataList.at(i)->volumeColor.redF()-1.0)*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue,
								1.0+((double)imageDataList.at(i)->volumeColor.greenF()-1.0)*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue,
								1.0+((double)imageDataList.at(i)->volumeColor.blueF()-1.0)*(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue
							);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMinThreshold, 
								1.0+((double)imageDataList.at(i)->volumeColor.redF()-1.0)*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue,
								1.0+((double)imageDataList.at(i)->volumeColor.greenF()-1.0)*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue,
								1.0+((double)imageDataList.at(i)->volumeColor.blueF()-1.0)*(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue
							);
						}
					}
				}
				if(imageDataList.at(i)->colormapName=="Black to"){
					//Black to
					imageDataList.at(i)->colorFun->RemoveAllPoints();
					if(imageDataList.at(i)->adjustColormap){
						if(!imageDataList.at(i)->invertColormap){
							imageDataList.at(i)->colorFun->AddRGBPoint((double)imageDataList.at(i)->currentMinThreshold, 0.0, 0.0, 0.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMaxThreshold, 
								imageDataList.at(i)->volumeColor.redF(),
								imageDataList.at(i)->volumeColor.greenF(),
								imageDataList.at(i)->volumeColor.blueF()
							);
						}else{
							imageDataList.at(i)->colorFun->AddRGBPoint((double)imageDataList.at(i)->currentMaxThreshold, 0.0, 0.0, 0.0);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMinThreshold, 
								imageDataList.at(i)->volumeColor.redF(),
								imageDataList.at(i)->volumeColor.greenF(),
								imageDataList.at(i)->volumeColor.blueF()
							);
						}
					}else{
						if(!imageDataList.at(i)->invertColormap){
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMinThreshold, 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.redF(), 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.greenF(), 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.blueF()
							);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMaxThreshold, 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.redF(), 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.greenF(), 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.blueF()
							);
						}else{
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMaxThreshold, 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.redF(), 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.greenF(), 
								(double)imageDataList.at(i)->currentMinThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.blueF()
							);
							imageDataList.at(i)->colorFun->AddRGBPoint(
								(double)imageDataList.at(i)->currentMinThreshold, 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.redF(), 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.greenF(), 
								(double)imageDataList.at(i)->currentMaxThreshold/(double)imageDataList.at(i)->maxValue*imageDataList.at(i)->volumeColor.blueF()
							);
						}
					}
				}
				if(imageDataList.at(i)->colormapName=="Color all to"){
					//Color all to
					imageDataList.at(i)->colorFun->RemoveAllPoints();
					imageDataList.at(i)->colorFun->AddRGBPoint(
						(double)imageDataList.at(i)->currentMinThreshold, 
						imageDataList.at(i)->volumeColor.redF(),
						imageDataList.at(i)->volumeColor.greenF(),
						imageDataList.at(i)->volumeColor.blueF()
					);
					imageDataList.at(i)->colorFun->AddRGBPoint(
						(double)imageDataList.at(i)->currentMaxThreshold,
						imageDataList.at(i)->volumeColor.redF(),
						imageDataList.at(i)->volumeColor.greenF(),
						imageDataList.at(i)->volumeColor.blueF()
					);
				}
				imageDataList.at(i)->colorFun->ClampingOff(); //Values out of the segment are zero
				imageDataList.at(i)->opacityFun->RemoveAllPoints();
				imageDataList.at(i)->opacityFun->AddSegment(imageDataList.at(i)->currentMinThreshold, 0, imageDataList.at(i)->currentMaxThreshold, 1.0);
				imageDataList.at(i)->opacityFun->ClampingOff(); //Values out of the segment are zero

				// THRESHOLD
				if(imageDataList.at(i)->isVolume){
					if(imageDataList.at(i)->dataFormat == dataType::DATA_JPEG){
						imageDataList.at(i)->luminance->SetInputConnection(imageDataList.at(i)->data->GetOutputPort());
						imageDataList.at(i)->extractVOI->SetInputConnection(imageDataList.at(i)->luminance->GetOutputPort());
					}else{
						imageDataList.at(i)->extractVOI->SetInputConnection(imageDataList.at(i)->data->GetOutputPort());
					}
					imageDataList.at(i)->extractVOI->SetVOI(
						(int)imageDataList.at(i)->VOI[0],(int)imageDataList.at(i)->VOI[1]-1,
						(int)imageDataList.at(i)->VOI[2],(int)imageDataList.at(i)->VOI[3]-1,
						(int)imageDataList.at(i)->VOI[4],(int)imageDataList.at(i)->VOI[5]-1
					);
					imageDataList.at(i)->threshold->SetInputConnection(imageDataList.at(i)->extractVOI->GetOutputPort());
					imageDataList.at(i)->threshold->ThresholdBetween((double)imageDataList.at(i)->currentMinThreshold, (double)imageDataList.at(i)->currentMaxThreshold);
					imageDataList.at(i)->threshold->ReplaceOutOn();
					imageDataList.at(i)->threshold->SetOutValue(0);
					imageDataList.at(i)->threshold->Update();
				}
			
				//PLANE
				imageDataList.at(i)->planeWidget->SetInteractor(iren);
				if(imageDataList.at(i)->planeIsVisible){
					imageDataList.at(i)->planeWidget->SetTextureVisibility(1);
					imageDataList.at(i)->planeWidget->GetMarginProperty()->SetOpacity(1);
					//imageDataList.at(i)->planeWidget->UpdatePlacement();//DEBUG
				}else{
					imageDataList.at(i)->planeWidget->SetTextureVisibility(0);
					imageDataList.at(i)->planeWidget->GetMarginProperty()->SetOpacity(0);
					//imageDataList.at(i)->planeWidget->UpdatePlacement();//DEBUG
				}
				if(imageDataList.at(i)->changePlaneInput){
					imageDataList.at(i)->changePlaneInput = false;
					if(imageDataList.at(i)->showPlane){
						if(imageDataList.at(i)->isVolume){
							imageDataList.at(i)->colorMap->SetLookupTable(imageDataList.at(i)->colorFun);
							imageDataList.at(i)->planeWidget->SetColorMap(imageDataList.at(i)->colorMap);
							imageDataList.at(i)->median->SetInputConnection(imageDataList.at(i)->threshold->GetOutputPort());
							if(imageDataList.at(i)->checkMedian){
								imageDataList.at(i)->median->SetKernelSize(imageDataList.at(i)->medianKernelX,imageDataList.at(i)->medianKernelY,imageDataList.at(i)->medianKernelZ);	
							}else{ 
								imageDataList.at(i)->median->SetKernelSize(1,1,1);
							}
							imageDataList.at(i)->planeWidget->SetInputConnection(imageDataList.at(i)->median->GetOutputPort());
						}else if(imageDataList.at(i)->isPolyData){
							imageDataList.at(i)->planeWidget->SetInputData(imageDataList.at(i)->poly);
						}
						if(imageDataList.at(i)->orientChanged){
							imageDataList.at(i)->orientChanged = false;
							imageDataList.at(i)->planeWidget->SetOrigin(-0.5*imageDataList.at(i)->width,-0.5*imageDataList.at(i)->height,-0.5*imageDataList.at(i)->depth);
							imageDataList.at(i)->planeWidget->SetPlaneOrientation(imageDataList.at(i)->orientIndex);	
						}
						if(false){
							imageDataList.at(i)->planeWidget->PlaceWidget(
								 imageDataList.at(i)->VOI[0]*settings.x_fac, imageDataList.at(i)->VOI[1]*settings.x_fac, 
								 imageDataList.at(i)->VOI[2]*settings.y_fac, imageDataList.at(i)->VOI[3]*settings.y_fac, 
								 imageDataList.at(i)->VOI[4]*settings.z_fac, imageDataList.at(i)->VOI[5]*settings.z_fac
							);
						}
						//SAVE LAST PLANE POSITION
						imageDataList.at(i)->planeWidget->GetOrigin(imageDataList.at(i)->planeOrigin);
						imageDataList.at(i)->planeOrientation = imageDataList.at(i)->planeWidget->GetPlaneOrientation();//TODO: check if neccessary or obsolete with "orientIndex"
						imageDataList.at(i)->planeWidget->GetPoint1(imageDataList.at(i)->planeP1);
						imageDataList.at(i)->planeWidget->GetPoint2(imageDataList.at(i)->planeP2);
						//SET NO MODIFIER FOR RIGHT MOUSE BUTTON
						imageDataList.at(i)->planeWidget->SetRightButtonAction(vtkImagePlaneWidget::VTK_CURSOR_ACTION);//TODO: unset if not current index
						imageDataList.at(i)->planeWidget->SetRightButtonAutoModifier(vtkImagePlaneWidget::VTK_NO_MODIFIER);//TODO: unset if not current index
						//SHOW PLANE & DISPLAY INFORMATION
						imageDataList.at(i)->planeWidget->UpdatePlacement();
						imageDataList.at(i)->planeWidget->DisplayTextOn();
						imageDataList.at(i)->planeWidget->On();
					}else{
						//SWITCH PLANE OFF
						imageDataList.at(i)->planeWidget->DisplayTextOff();
						imageDataList.at(i)->planeWidget->Off();
					}
				}
				//IMPLICIT PLANE
				/*
				imageDataList.at(i)->implicitPlane->SetInteractor(iren);
				imageDataList.at(i)->implicitPlane->SetInputConnection(imageDataList.at(i)->threshold->GetOutputPort());
				imageDataList.at(i)->implicitPlane->SetOrigin(
					0.5*(imageDataList.at(i)->VOI[1]-imageDataList.at(i)->VOI[0]),
					0.5*(imageDataList.at(i)->VOI[3]-imageDataList.at(i)->VOI[2]),
					0.5*(imageDataList.at(i)->VOI[5]-imageDataList.at(i)->VOI[4])
				);
				imageDataList.at(i)->implicitPlane->SetOutlineTranslation(1);
				imageDataList.at(i)->implicitPlane->Off();
				*/
			}
			//VOLUME
			if(imageDataList.at(i)->isVolume){
				imageDataList.at(i)->volumeMapper->RemoveAllClippingPlanes();//DEBUG
				if(imageDataList.at(i)->fileChanged){
					imageDataList.at(i)->fileChanged = false;
					imageDataList.at(i)->volume->GetProperty()->SetColor(imageDataList.at(i)->colorFun);
					imageDataList.at(i)->volume->GetProperty()->SetScalarOpacity(imageDataList.at(i)->opacityFun);
					imageDataList.at(i)->volume->GetProperty()->SetInterpolationTypeToLinear();
				}
				if(imageDataList.at(i)->showPlane){
					imageDataList.at(i)->clipPlane->SetOrigin(imageDataList.at(i)->planeWidget->GetOrigin());
					if(imageDataList.at(i)->switchPlane){
						imageDataList.at(i)->clipPlane->SetNormal(
							(-1)*imageDataList.at(i)->planeWidget->GetNormal()[0],
							(-1)*imageDataList.at(i)->planeWidget->GetNormal()[1],
							(-1)*imageDataList.at(i)->planeWidget->GetNormal()[2]
						);
					}else{
						imageDataList.at(i)->clipPlane->SetNormal(imageDataList.at(i)->planeWidget->GetNormal());
					}
					imageDataList.at(i)->planeCollection->AddItem(imageDataList.at(i)->clipPlane);
					imageDataList.at(i)->volumeMapper->SetClippingPlanes(imageDataList.at(i)->planeCollection);
				}
				switch(imageDataList.at(i)->blendMode){
					case 1:  imageDataList.at(i)->volumeMapper->SetBlendModeToComposite(); break;
					case 2:  imageDataList.at(i)->volumeMapper->SetBlendModeToAdditive(); break;
					case 3:  imageDataList.at(i)->volumeMapper->SetBlendModeToMinimumIntensity(); break;
					default: imageDataList.at(i)->volumeMapper->SetBlendModeToMaximumIntensity(); break;
				}
				imageDataList.at(i)->volumeMapper->SetInputConnection(imageDataList.at(i)->threshold->GetOutputPort());
				imageDataList.at(i)->volumeMapper->SetFinalColorLevel(1.0-imageDataList.at(i)->objectOpacity);
				imageDataList.at(i)->volume->SetMapper(imageDataList.at(i)->volumeMapper);
				imageDataList.at(i)->volume->SetScale(settings.x_fac,settings.y_fac,settings.z_fac);
				imageDataList.at(i)->transform->Translate(imageDataList.at(i)->shift);
				imageDataList.at(i)->transform->RotateX(imageDataList.at(i)->rot[0]);
				imageDataList.at(i)->transform->RotateY(imageDataList.at(i)->rot[1]);
				imageDataList.at(i)->transform->RotateZ(imageDataList.at(i)->rot[2]);
				imageDataList.at(i)->volume->SetUserTransform(imageDataList.at(i)->transform);
				imageDataList.at(i)->volume->Update();
				renderer->AddVolume(imageDataList.at(i)->volume);
				if(imageDataList.at(i)->showObject && imageDataList.at(i)->isVolume){
					imageDataList.at(i)->volume->SetVisibility(true);
				}else{
					imageDataList.at(i)->volume->SetVisibility(false);
				}
			//POLYDATA
			}else if(imageDataList.at(i)->isPolyData){
				imageDataList.at(i)->polyMapper->SetInputData(imageDataList.at(i)->poly);
				imageDataList.at(i)->polyActor->SetMapper(imageDataList.at(i)->polyMapper);
				imageDataList.at(i)->polyActor->SetScale(settings.x_fac,settings.y_fac,settings.z_fac);
				switch(imageDataList.at(i)->polyMode){
					case 0: default:
						imageDataList.at(i)->polyActor->GetProperty()->SetRepresentationToPoints();
						imageDataList.at(i)->polyActor->GetProperty()->SetPointSize(imageDataList.at(i)->pointSize);
						break;
					case 1:
						imageDataList.at(i)->polyActor->GetProperty()->SetRepresentationToWireframe();
						break;
					case 2:
						imageDataList.at(i)->polyActor->GetProperty()->SetRepresentationToSurface();
						break;
				}
				imageDataList.at(i)->polyActor->GetProperty()->SetOpacity(imageDataList.at(i)->objectOpacity);
				imageDataList.at(i)->polyActor->GetProperty()->SetColor(
					imageDataList.at(i)->polyColor.redF(),
					imageDataList.at(i)->polyColor.greenF(),
					imageDataList.at(i)->polyColor.blueF()
				);
				renderer->AddActor(imageDataList.at(i)->polyActor);
				if(imageDataList.at(i)->showObject && imageDataList.at(i)->isPolyData){
					imageDataList.at(i)->polyActor->SetVisibility(true);
				}else{
					imageDataList.at(i)->polyActor->SetVisibility(false);
				}
			}
		}
	}
	if(settings.oneFileLoaded){
		//CUBE AXES
		if(settings.showAxesTriad || settings.showAxesBox){
			if(settings.showAxesTriad){
				axes->SetXAxisLabelVisibility(1);
				axes->SetYAxisLabelVisibility(1);
				axes->SetZAxisLabelVisibility(1);
				axes->SetXAxisMinorTickVisibility(1);
				axes->SetYAxisMinorTickVisibility(1);
				axes->SetZAxisMinorTickVisibility(1);
				axes->SetXAxisTickVisibility(1);
				axes->SetYAxisTickVisibility(1);
				axes->SetZAxisTickVisibility(1);
				axes->SetXLabelFormat("%6.1f");
				axes->SetYLabelFormat("%6.1f");
				axes->SetZLabelFormat("%6.1f");
				axes->SetScreenSize(12.0);
				axes->SetFlyModeToOuterEdges();
				axes->SetCornerOffset(0.0);
			}else{
				axes->SetXAxisLabelVisibility(0);
				axes->SetYAxisLabelVisibility(0);
				axes->SetZAxisLabelVisibility(0);
				axes->SetXAxisMinorTickVisibility(0);
				axes->SetYAxisMinorTickVisibility(0);
				axes->SetZAxisMinorTickVisibility(0);
				axes->SetXAxisTickVisibility(0);
				axes->SetYAxisTickVisibility(0);
				axes->SetZAxisTickVisibility(0);
				axes->SetXLabelFormat("%6.1f");
				axes->SetYLabelFormat("%6.1f");
				axes->SetZLabelFormat("%6.1f");
				axes->SetFlyModeToStaticEdges();
			}		
			if(activeImageData->isVolume)
				axes->SetBounds(activeImageData->volume->GetBounds());
			else if(activeImageData->isPolyData)
				axes->SetBounds(activeImageData->poly->GetBounds());
			axes->SetCamera(renderer->GetActiveCamera());
			axes->SetRebuildAxes(true);
			renderer->RemoveActor(axes);
			renderer->AddActor(axes);
		}else{
			renderer->RemoveActor(axes);
		}

		//SCALARBAR
		if(settings.showScalarBar && activeImageData->isVolume){
			scalarBarActor->SetLookupTable(activeImageData->colorFun);
			scalarBarActor->SetTitle("Intensity");
			scalarBarActor->SetMaximumWidthInPixels(100);
			scalarBarActor->SetMaximumHeightInPixels(700);
			scalarBarWidget->SetInteractor(iren);
			scalarBarWidget->SetScalarBarActor(scalarBarActor);
			scalarBarWidget->On();
			renderer->AddActor(scalarBarActor);
		}else{
			scalarBarWidget->Off();
			renderer->RemoveActor(scalarBarActor);
		}
		//DISPLAY AXES
		orientWidget->SetOrientationMarker(axesActor);
		orientWidget->SetInteractor(iren);
		orientWidget->SetViewport(0.0, 0.0, 0.3, 0.3);
		if(settings.showOrientAxes){
			orientWidget->SetEnabled(1);
		}else{
			orientWidget->SetEnabled(0);
		}
		//RESET CAM
		if(settings.firstFileLoaded){
			settings.firstFileLoaded = false;
			slotFrontZ();
		}
		//RENDER SCENE
		renWin->Render();
	}
}

void OCTview3R::slotSetColormap(QString value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->colormapName = value;

		//BUG WORKAROUND:
		//TODO: Move this to ImageData
		renderer->RemoveViewProp(activeImageData->volume);
		renderer->RemoveVolume(activeImageData->volume);

		connectVTKPipeline();
	}
}
void OCTview3R::slotPickVolumeColor()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		QColor color = QColorDialog::getColor(activeImageData->volumeColor, this);
		if(color.isValid()){
			activeImageData->volumeColor = color;
			this->ui->pushButton_pickVolumeColor->setStyleSheet("background-color: "+activeImageData->volumeColor.name());
			//BUG WORKAROUND:
			//TODO: Change this to PolyData and move to ImageData
			renderer->RemoveViewProp(activeImageData->volume);
			renderer->RemoveVolume(activeImageData->volume);
			connectVTKPipeline();
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
			connectVTKPipeline();
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
		connectVTKPipeline();
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
		connectVTKPipeline();
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
		connectVTKPipeline();
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
		connectVTKPipeline();
	}
}
void OCTview3R::slotAdjustColormap(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->adjustColormap = value;
		slotSetThreshold();
		connectVTKPipeline();
	}
}
void OCTview3R::slotInvertColormap(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->invertColormap = value;
		slotSetThreshold();
		connectVTKPipeline();
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
		connectVTKPipeline();
	}
}
void OCTview3R::slotKernelXChanged(int value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->medianKernelX = value;
	}
	slotMedianCheckBox(activeImageData->checkMedian);
}
void OCTview3R::slotKernelYChanged(int value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->medianKernelY = value;
	}
	slotMedianCheckBox(activeImageData->checkMedian);
}
void OCTview3R::slotKernelZChanged(int value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->medianKernelZ = value;
	}
	slotMedianCheckBox(activeImageData->checkMedian);
}
void OCTview3R::slotOrientationChanged(int value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->changePlaneInput = true;
		activeImageData->orientChanged = true;
		activeImageData->orientIndex = value;
		connectVTKPipeline();
	}
}
void OCTview3R::slotGetPlaneData()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		double aOrigin[3]; 
		double *origin = aOrigin; 
		origin	= activeImageData->planeWidget->GetOrigin();
		double aCenter[3]; 
		double *center = aCenter; 
		center = activeImageData->planeWidget->GetCenter();
		double aNormal[3]; 
		double *normal = aNormal; 
		activeImageData->planeWidget->GetNormal(normal);
		this->ui->planeLineEdit->setText("normal:" + QString().number(normal[0],'g',4) + ":" + QString().number(normal[1],'g',4) + ":" + QString().number(normal[2],'g',4) + 
									  " | center:" + QString().number(center[0],'g',4) + ":" + QString().number(center[1],'g',4) + ":" + QString().number(center[2],'g',4));
	}else{
		this->ui->planeLineEdit->clear();
	}
}
void OCTview3R::slotX0(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		if(this->ui->x0DoubleSpinBox->value() < activeImageData->VOI[1]) //-1
			activeImageData->VOI[0] = this->ui->x0DoubleSpinBox->value();
		else{
			QMessageBox::information(this,tr("ERROR"), tr("VOI value x_start out of bounds"));
			this->ui->x0DoubleSpinBox->setValue(activeImageData->VOI[0]);
		}

	}
}
void OCTview3R::slotX1(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		if(this->ui->x1DoubleSpinBox->value() >= activeImageData->VOI[0]) //+1
			activeImageData->VOI[1] = this->ui->x1DoubleSpinBox->value();
		else{ 
			QMessageBox::information(this,tr("ERROR"), tr("VOI value x_end out of bounds"));
			ui->x1DoubleSpinBox->setValue(activeImageData->VOI[1]);
		}
	}
}
void OCTview3R::slotY0(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		if(this->ui->y0DoubleSpinBox->value() < activeImageData->VOI[3]) //-1
			activeImageData->VOI[2] = this->ui->y0DoubleSpinBox->value();
		else{
			QMessageBox::information(this,tr("ERROR"), tr("VOI value y_start out of bounds"));
			this->ui->y0DoubleSpinBox->setValue(activeImageData->VOI[2]);
		}
	}
}
void OCTview3R::slotY1(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		if(this->ui->y1DoubleSpinBox->value() >= activeImageData->VOI[2]) //+1
			activeImageData->VOI[3] = this->ui->y1DoubleSpinBox->value();
		else{
			QMessageBox::information(this,tr("ERROR"), tr("VOI value y_end out of bounds"));
			this->ui->y1DoubleSpinBox->setValue(activeImageData->VOI[3]);
		}
	}
}
void OCTview3R::slotZ0(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		if(this->ui->z0DoubleSpinBox->value() < activeImageData->VOI[5]) //-1
			activeImageData->VOI[4] = this->ui->z0DoubleSpinBox->value();
		else{
			QMessageBox::information(this,tr("ERROR"), tr("VOI value z_start out of bounds"));
			this->ui->z0DoubleSpinBox->setValue(activeImageData->VOI[4]);
		}
	}
}
void OCTview3R::slotZ1(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		if(this->ui->z1DoubleSpinBox->value() >= activeImageData->VOI[4]) //+1
			activeImageData->VOI[5] = this->ui->z1DoubleSpinBox->value();
		else{
			QMessageBox::information(this,tr("ERROR"), tr("VOI value z_end out of bounds"));
			this->ui->z1DoubleSpinBox->setValue(activeImageData->VOI[5]);
		}
	}
}
void OCTview3R::slotRotX(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->fileChanged = true;
		activeImageData->rot[0] = value;
	}
}
void OCTview3R::slotRotY(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->fileChanged = true;
		activeImageData->rot[1] = value;
	}
}
void OCTview3R::slotRotZ(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->fileChanged = true;
		activeImageData->rot[2] = value;
	}
}
void OCTview3R::slotShiftX(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->fileChanged = true;
		activeImageData->shift[0] = value;
	}
}
void OCTview3R::slotShiftY(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->fileChanged = true;
		activeImageData->shift[1] = value;
	}
}
void OCTview3R::slotShiftZ(double value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->fileChanged = true;
		activeImageData->shift[2] = value;
	}
}
void OCTview3R::slotRenderAgain()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->fileChanged = true;
		activeImageData->changePlaneInput = true;

		//QFUTURE to load the volume in another thread while still responsive in the GUI with a ProgressDialog
		QFutureWatcher<void>* watcherLoadFile = new QFutureWatcher<void>();
		QFuture<void> futureLoadFile;
		QProgressDialog* progressDialog = new QProgressDialog("Loading file...", QString(), 0, 0, this);
		progressDialog->setMinimumWidth(600);
		connect(watcherLoadFile, SIGNAL(finished()), progressDialog, SLOT(close()));
		connect(progressDialog, SIGNAL(canceled()), watcherLoadFile, SLOT(cancel()));
		connect(watcherLoadFile, SIGNAL(finished()), this, SLOT(connectVTKPipeline()));
		watcherLoadFile->setFuture(futureLoadFile);
		progressDialog->show();
	}
}
void OCTview3R::slotPlaneUp()
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		double aNormal[3]; 
		double *normal = aNormal; 
		normal = activeImageData->planeWidget->GetNormal();
		if((normal[0]==1) || (normal[1]==1) || (normal[2]==1) || (normal[0]==-1) || (normal[1]==-1) || (normal[2]==-1)){
			int index = activeImageData->planeWidget->GetSliceIndex();
			activeImageData->planeWidget->SetSliceIndex(index + 1);
			iren->Render();
			connectVTKPipeline();
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
		if((normal[0]==1) || (normal[1]==1) || (normal[2]==1) || (normal[0]==-1) || (normal[1]==-1) || (normal[2]==-1)){
			int index = activeImageData->planeWidget->GetSliceIndex();
			activeImageData->planeWidget->SetSliceIndex(index - 1);
			iren->Render();
			connectVTKPipeline();
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
		connectVTKPipeline();
	}
}

void OCTview3R::slotSetObjectOpacity(int value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->objectOpacity = (double)value * 0.01;
		connectVTKPipeline();
	}
}

void OCTview3R::slotSetBlendMode(int index)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->blendMode = index;
		connectVTKPipeline();
	}
}
void OCTview3R::slotSetPolyMode(int index)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->polyMode = index;
		connectVTKPipeline();
	}
}
void OCTview3R::slotPlaneVisibility(bool value)
{
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->planeIsVisible = value;
		connectVTKPipeline();
	}
}
void OCTview3R::slotFrontX()
{
	cam = renderer->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(0,1,0);
	cam->SetPosition(-1,0,0);
	slotResetCam();
}
void OCTview3R::slotFrontY()
{
	cam = renderer->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(0,0,1);
	cam->SetPosition(0,-1,0);
	slotResetCam();
}
void OCTview3R::slotFrontZ()
{
	cam = renderer->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(1,0,0);
	cam->SetPosition(0,0,-1);
	slotResetCam();
}
void OCTview3R::slotBackX()
{
	cam = renderer->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(0,1,0);
	cam->SetPosition(1,0,0);
	slotResetCam();
}
void OCTview3R::slotBackY()
{
	cam = renderer->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);	
	cam->SetViewUp(0,0,1);
	cam->SetPosition(0,1,0);
	slotResetCam();
}
void OCTview3R::slotBackZ()
{
	cam = renderer->GetActiveCamera();
	cam->SetFocalPoint(0,0,0);
	cam->SetViewUp(1,0,0);
	cam->SetPosition(0,0,1);
	slotResetCam();
}
void OCTview3R::slotRot90()
{
	cam = renderer->GetActiveCamera();
	double roll = cam->GetRoll();
	cam->SetRoll(roll - 90.0);
	slotResetCam();
}
void OCTview3R::slotScaleX(double value)
{		
	settings.x_fac = value;
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->changePlaneInput = true;
		connectVTKPipeline();
	}
}
void OCTview3R::slotScaleY(double value)
{
	settings.y_fac = value;
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->changePlaneInput = true;
		connectVTKPipeline();
	}
}
void OCTview3R::slotScaleZ(double value)
{
	settings.z_fac = value;
	if(settings.oneFileLoaded && activeImageData->fileLoaded){
		activeImageData->changePlaneInput = true;
		connectVTKPipeline();
	}
}
//TODO: check if callback works when camera is rotated
void OCTview3R::getCamRot()
{
	cam = renderer->GetActiveCamera();
	double focalPoint[3];
	cam->GetFocalPoint(focalPoint);
	double viewUp[3];
	cam->GetViewUp(viewUp);
	double position[3];
	cam->GetPosition(position);

	double rotX = 0, rotY = 0, rotZ = 0;

	//TODO: calculate x-,y-,z-angles

	ui->rotXCamDoubleSpinBox->setValue(rotX);
	ui->rotYCamDoubleSpinBox->setValue(rotY);
	ui->rotZCamDoubleSpinBox->setValue(rotZ);
}
void OCTview3R::slotRotCamX(double value)
{
	double dx = value - settings.x_rot_cam;
	settings.x_rot_cam = value;
	camTrans->RotateX(dx);
	camTrans->RotateY(0.0);
	camTrans->RotateZ(0.0);
	cam = renderer->GetActiveCamera();
	cam->ApplyTransform(camTrans);
	slotResetCam();
}
void OCTview3R::slotRotCamY(double value)
{
	double dy = value - settings.y_rot_cam;
	settings.y_rot_cam = value;
	camTrans->RotateX(0.0);
	camTrans->RotateY(dy);
	camTrans->RotateZ(0.0);
	cam = renderer->GetActiveCamera();
	cam->ApplyTransform(camTrans);
	slotResetCam();
}
void OCTview3R::slotRotCamZ(double value)
{
	double dz = value - settings.z_rot_cam;
	settings.z_rot_cam = value;
	camTrans->RotateX(0.0);
	camTrans->RotateY(0.0);
	camTrans->RotateZ(dz);
	cam = renderer->GetActiveCamera();
	cam->ApplyTransform(camTrans);
	slotResetCam();
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
	settings.background_RGB1[0] = qstr.split("-")[0].toInt();
	settings.background_RGB1[1] = qstr.split("-")[1].toInt();
	settings.background_RGB1[2] = qstr.split("-")[2].toInt();
	initializeVTKPipeline();
}
void OCTview3R::slotBackground2(QString qstr)
{
	settings.background_RGB2[0] = qstr.split("-")[0].toInt();
	settings.background_RGB2[1] = qstr.split("-")[1].toInt();
	settings.background_RGB2[2] = qstr.split("-")[2].toInt();
	initializeVTKPipeline();
}
void OCTview3R::slotSaveDisplay()
{
	vtkWindowToImageFilter *w2i = vtkWindowToImageFilter::New();
	vtkTIFFWriter *writer = vtkTIFFWriter::New();
	w2i->SetInput(renWin);
	w2i->Update();
	writer->SetInputConnection(w2i->GetOutputPort());
	QFileDialog getFileDialog(this, "Save display as...");
	getFileDialog.setNameFilter("Image file (*.tif)");
	getFileDialog.setAcceptMode(QFileDialog::AcceptSave);	
	if(getFileDialog.exec() == QFileDialog::Accepted){
		writer->SetFileName(getFileDialog.selectedFiles()[0].toStdString().c_str());
		renWin->Render();
		writer->Write();
	}else{
		QMessageBox::information(this,tr("WARNING"), tr("No image was saved"));	
	}
}
void OCTview3R::slotCloseTab()
{
	//TODO: Still messy. First tab can't be removed, volume view is not updated!
	int index = imageDataList.indexOf(activeImageData,1);
	if(imageDataList.length()>1 && (index!=-1)){
		this->ui->tabWidget->removeTab(index);
		imageDataList.removeOne(activeImageData);
		this->ui->tabWidget->setCurrentIndex(index+1);
	}
}

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