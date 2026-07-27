#ifndef OCTVIEW3R_H
#define OCTVIEW3R_H

#include "vtkSmartPointer.h" // Required for smart pointer internal ivars.
#include "dataTypes.h"
#include "opendata.h"
#include "openpoly.h"
#include <QMainWindow>
#include <QColorDialog>
#include "qmath.h"
#include "qvector.h"
#include "qlabel.h"

// Forward Qt class declarations
class Ui_OCTview3R;
class OpenData;
class OpenPolyData;

// Forward VTK class declarations
class vtkImageReader2;
class vtkRenderer;
class vtkVolume;
class vtkRenderWindow;
class vtkObject;
class vtkRenderWindowInteractor;
class vtkImageThreshold;
class vtkImageLuminance;
class vtkColorTransferFunction;
class vtkPiecewiseFunction;
class vtkVolumeProperty;
class vtkSmartVolumeMapper;
class vtkImagePlaneWidget;
class vtkImageReader;
class vtkImageResample;
class vtkImageMedian3D;
class vtkTIFFReader;
class vtkJPEGReader;
class vtkImageResliceMapper;
class vtkImageProperty;
class vtkImageSlice;
class vtkCamera;
class vtkTransform;
class vtkScalarBarActor;
class vtkScalarBarWidget;
class vtkCubeAxesActor;
class RangeSlider;
class vtkLookupTable;
class vtkImageMapToColors;
class vtkMath;
class vtkEventQtSlotConnect;
class vtkExtractVOI;
class vtkOrientationMarkerWidget;
class vtkAxesActor;
class vtkWindowToImageFilter;
class vtkTIFFWriter;
class vtkVertexGlyphFilter;
class vtkDataSetAttributes;
class vtkPlane;
class vtkImplicitPlaneWidget;
class vtkClipVolume;
class vtkClipPolyData;
class vtkImageClip;
class vtkCutter;
class vtkVolumeRayCastMapper;
class vtkVolumeRayCastMIPFunction;
class vtkVolumeRayCastCompositeFunction;
class vtkPlaneSource;
class vtkPlaneCollection;
//POINT
class vtkPolyData;
class vtkPolyDataMapper;
class vtkPolyDataReader;
class vtkPoints;
class vtkPointData;
class vtkCellArray;
class vtkUnsignedCharArray;
class vtkActor;
//MESH
class vtkDataSetMapper;
class vtkPolygon;
class vtkCleanPolyData;
class vtkDelaunay3D;
class vtkXMLPolyDataReader;
class vtkXMLImageDataReader;
class vtkDelaunay2D;
//DATA
class vtkGenericDataObjectReader;
class vtkStructuredGrid;
class vtkStructuredGridReader;
class vtkStructuredPointsReader;
class vtkStructuredGridGeometryFilter;
class vtkUnstructuredGrid;
class vtkUnstructuredGridReader;
class vtkImageDataGeometryFilter;
class vtkVolumeMapper;
class vtkTransform;
class vtkTransformFilter;
class vtkTransformPolyDataFilter;
class vtkMatrix4x4;
class vtkAlgorithm;
class vtkAlgorithmOutput;
class vtkWarpScalar;
class vtkCubeAxesActor2D;
class vtkAxisActor2D;
class vtkInteractorStyleTrackballCamera;
class vtkInteractorStyleTrackball;
class vtkImageReslice;
class vtkImageResliceMapper;

struct ImageData
{
	//data and polydata
	vtkSmartPointer<vtkImageThreshold> threshold;
	vtkSmartPointer<vtkColorTransferFunction> colorFun;
	vtkSmartPointer<vtkPiecewiseFunction> opacityFun;
	vtkSmartPointer<vtkImagePlaneWidget> planeWidget;
	vtkSmartPointer<vtkImageMedian3D> median;
	vtkSmartPointer<vtkExtractVOI> extractVOI;
	vtkSmartPointer<vtkVolume> volume;
	vtkSmartPointer<vtkActor> actor;
	vtkSmartPointer<vtkTransform> transform; //https://gitlab.kitware.com/vtk/vtk/-/issues/17140
	vtkSmartPointer<vtkImageLuminance> luminance;
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper;
	vtkSmartPointer<vtkPolyDataMapper> polyMapper;
	vtkSmartPointer<vtkActor> polyActor;
	vtkSmartPointer<vtkImageMapToColors> colorMap;
	vtkSmartPointer<vtkPlane> clipPlane;
	vtkSmartPointer<vtkImageData> image;
	vtkSmartPointer<vtkImplicitPlaneWidget> implicitPlane;
	vtkSmartPointer<vtkPlaneCollection> planeCollection;

	//object
	vtkAlgorithm *object;
	vtkAlgorithmOutput *output;
	QString typeName;
	bool fileLoaded; //always true after the first loaded file
	bool fileChanged;
	QString fileName;
	QString filePath;
	bool isPolyData;
	bool isVolume;
	vtkImageReader2 *data;
	vtkPolyData *poly;
	dataType dataFormat;
	polyType polyFormat;
	endianType endian;
	bitsizeType bitsize;
	int width;
	int height;
	int depth;
	double VOI[6];
	double rot[3];
	double shift[3];
	double spacing[3];
	double objectOpacity;
	bool showObject;

	//color
	QString colormapName; //0-Greyscale, 1-Rainbow, 2-Dark Body
	bool adjustColormap;
	bool invertColormap;
	int blendMode;
	int polyMode;
	QColor polyColor;
	QColor volumeColor;
	double colorRGB[3];
	int pointSize;

	//threshold
	bool changedThreshold;
	int minValue;
	int maxValue;
	int currentMinThreshold;
	int currentMaxThreshold;

	//plane
	bool initPlane;
	bool showPlane;
	double planeOrigin[3];
	double planeP1[3];
	double planeP2[3];
	bool checkMedian;
	int medianKernelX;
	int medianKernelY;
	int medianKernelZ;
	int orientIndex; //0: xy, 1:xz, 2:yz
	bool orientChanged;
	int planeOrientation;
	bool changePlaneInput;
	bool switchPlane;
	bool planeIsVisible;
};

struct Settings
{
	//current index
	int activeIndex;
	int numMaxIndex;

	//init questions
	bool oneFileLoaded;
	bool firstFileLoaded;

	//general
	int background_RGB1[3];
	int background_RGB2[3];

	//actions
	bool showAxesTriad;
	bool showAxesBox;
	bool showOrientAxes;
	bool showScalarBar;

	//stretching
	double x_fac;
	double y_fac;
	double z_fac;

	//camera rotation
	double x_rot_cam;
	double y_rot_cam;
	double z_rot_cam;
};

class OCTview3R : public QMainWindow
{
	Q_OBJECT

public:
	//con-/destructor
	OCTview3R();
	~OCTview3R();

public slots:
	virtual void slotProcessDataFile();
	virtual void slotProcessPolyFile();
	virtual void slotExit();

protected:
	vtkRenderer* renderer;
	vtkRenderWindow* renWin;
	vtkRenderWindowInteractor* iren;
	//vtkVolumeRayCastCompositeFunction *rayCastCompositeFunction;
	//vtkVolumeRayCastMIPFunction *rayCastMIPFunction;
	//vtkVolumeRayCastMapper *volumeRayCastMapper;
	vtkImageReader* imageReader;
	vtkTIFFReader* TIFFReader;
	vtkJPEGReader* JPEGReader;
	vtkStructuredPointsReader *VTKReader;
	vtkScalarBarActor* scalarBarActor;
	vtkScalarBarWidget* scalarBarWidget;
	vtkCubeAxesActor* axes;
	vtkLookupTable* lookupTable;
	vtkImageResliceMapper *im;
	vtkImageProperty *ip;
	vtkImageSlice *ia;
	vtkCamera *cam;
	vtkTransform *camTrans;
	vtkMath *math;
	vtkEventQtSlotConnect *connections;
	vtkAxesActor *axesActor;
	vtkOrientationMarkerWidget *orientWidget;

signals:
	void signalLoadFileStarted(void);
	void signalLoadFileFinished(void);

protected slots:
	void connectVTKPipeline(void);

	//POLY TESTING
	void magic();
	void transform();

	//new file
	void slotSetImageData(ImageData*);
	void slotSetImageData(int);
	void slotOpenDataFileDialog(void);
	void slotOpenPolyFileDialog(void);
	void slotDataFileDialogClosed(vtkImageReader2*);
	void slotPolyFileDialogClosed(vtkPolyData*);

	//threshold
	void slotSetThreshold(void);
	void slotCheckMinThresholdSlider(int);
	void slotCheckMaxThresholdSlider(int);

	//color mapping
	void slotSetColormap(QString);
	void slotAdjustColormap(bool);
	void slotSetBlendMode(int);
	void slotSetPolyMode(int);
	void slotInvertColormap(bool);
	void slotPickVolumeColor(void);
	void slotPickPolyColor(void);
	void slotSetObjectOpacity(int);
	void slotSetPointSize(int);

	//object
	void slotShowObject(bool);
	void slotX0(double);
	void slotX1(double);
	void slotY0(double);
	void slotY1(double);
	void slotZ0(double);
	void slotZ1(double);
	void slotRotX(double);
	void slotRotY(double);
	void slotRotZ(double);
	void slotShiftX(double);
	void slotShiftY(double);
	void slotShiftZ(double);

	//plane
	void slotShowPlane(bool);
	void slotPlaneUp(void);
	void slotPlaneDown(void);
	void slotMedianCheckBox(bool);
	void slotKernelXChanged(int);
	void slotKernelYChanged(int);
	void slotKernelZChanged(int);
	void slotOrientationChanged(int);
	void slotGetPlaneData(void);
	void slotCheckFlipPlane(bool);
	void slotPlaneVisibility(bool);
 
	//scale and axis
	void slotShowScalarBar(bool);
	void slotShowAxesTriad(bool);
	void slotShowOrientAxes(bool);
	void slotShowAxesBox(bool);

	//general slots
	void slotRenderAgain(void);
	void slotFrontX(void);
	void slotFrontY(void);
	void slotFrontZ(void);
	void slotBackX(void);
	void slotBackY(void);
	void slotBackZ(void);
	void slotRot90(void);
	void slotScaleX(double);
	void slotScaleY(double);
	void slotScaleZ(double);
	void slotRotCamX(double);
	void slotRotCamY(double);
	void slotRotCamZ(double);
	void slotRotCamStepX(double);
	void slotRotCamStepY(double);
	void slotRotCamStepZ(double);
	void slotResetCam(void);
	void slotBackground1(QString);
	void slotBackground2(QString);
	void slotSaveDisplay(void);
	void slotCloseTab();

private:
	//callback variables
	static void moveCameraCallbackFunction(
		vtkObject *caller,
		unsigned long eventId,
		void *clientData,
		void *callData
	);
	bool moveCameraCallbackMutex;
	static void onePlaneCallbackFunction(
		vtkObject *caller,
		unsigned long eventId,
		void *clientData,
		void *callData
	);
	bool onePlaneCallbackMutex;

	//essential variables
	QString fileName;
	QString filePath;
	QLabel *statusLabel;

	//designer form
	Ui_OCTview3R *ui;
	QWidget *tab;
	OpenData *openData;
	OpenPoly *openPoly;

	//parameter structures
	Settings settings;
	ImageData* activeImageData;
	QList<ImageData*> imageDataList;

public:
	//helper
	ImageData* getInitializedImageData();
	void calculateThickSlabDistanceRange();
	void getCamRot();
	void initializeVTKPipeline();
};

#endif // OCTVIEW3R_H
