#pragma once

#include "dataTypes.h"

#include <QColor>
#include <QString>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkBox;
class vtkClipPolyData;
class vtkColorTransferFunction;
class vtkExtractVOI;
class vtkImageAppendComponents;
class vtkImageData;
class vtkImageExtractComponents;
class vtkImageLuminance;
class vtkImageMapToColors;
class vtkImageMask;
class vtkImageMedian3D;
class vtkImagePlaneWidget;
class vtkImageShiftScale;
class vtkImageThreshold;
class vtkImplicitPlaneWidget;
class vtkPiecewiseFunction;
class vtkPlane;
class vtkPlaneCollection;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkPolyDataNormals;
class vtkSmartVolumeMapper;
class vtkTransform;
class vtkVolume;

struct ImageData
{
	vtkSmartPointer<vtkImageThreshold> threshold;
	vtkSmartPointer<vtkColorTransferFunction> colorFun;
	vtkSmartPointer<vtkPiecewiseFunction> opacityFun;
	vtkSmartPointer<vtkImagePlaneWidget> planeWidget;
	vtkSmartPointer<vtkImageMedian3D> median;
	vtkSmartPointer<vtkExtractVOI> extractVOI;
	vtkSmartPointer<vtkImageExtractComponents> rgbComponents;
	vtkSmartPointer<vtkVolume> volume;
	vtkSmartPointer<vtkActor> actor;
	vtkSmartPointer<vtkTransform> transform;
	vtkSmartPointer<vtkImageLuminance> luminance;
	vtkSmartPointer<vtkImageThreshold> rgbMaskThreshold;
	vtkSmartPointer<vtkImageShiftScale> rgbWindowLevel;
	vtkSmartPointer<vtkImageMask> rgbMask;
	vtkSmartPointer<vtkImageAppendComponents> rgbaVolume;
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper;
	vtkSmartPointer<vtkPolyDataMapper> polyMapper;
	vtkSmartPointer<vtkPolyDataNormals> polyNormals;
	vtkSmartPointer<vtkActor> polyActor;
	vtkSmartPointer<vtkBox> polyClipBox;
	vtkSmartPointer<vtkClipPolyData> polyClipper;
	vtkSmartPointer<vtkImageMapToColors> colorMap;
	vtkSmartPointer<vtkPlane> clipPlane;
	vtkSmartPointer<vtkImageData> image;
	vtkSmartPointer<vtkImplicitPlaneWidget> implicitPlane;
	vtkSmartPointer<vtkPlaneCollection> planeCollection;
	vtkSmartPointer<vtkPolyData> poly;

	QString typeName;
	QString fileName;
	QString filePath;
	bool fileLoaded = false;
	bool fileChanged = false;
	bool isPolyData = false;
	bool isVolume = false;
	dataType dataFormat = DATA_UNDEF;
	polyType polyFormat = POLY_UNDEF;
	endianType endian = ENDIAN_UNDEF;
	bitsizeType bitsize = BIT_UNDEF;
	int width = 0;
	int height = 0;
	int depth = 0;
	double VOI[6] = {};
	double sourceVOI[6] = {};
	double rot[3] = {};
	double shift[3] = {};
	double scale[3] = { 1.0, 1.0, 1.0 };
	double spacing[3] = { 1.0, 1.0, 1.0 };
	unsigned int spacingInMillimetresMask = 0;
	double objectOpacity = 1.0;
	double polyGloss = 0.1;
	bool showObject = false;

	QString colormapName = "Greyscale";
	bool adjustColormap = true;
	bool invertColormap = false;
	bool renderRgb = false;
	int blendMode = 0;
	int polyMode = 0;
	QColor polyColor = QColor(255, 0, 0);
	QColor volumeColor = QColor(255, 0, 0);
	int pointSize = 1;

	int minValue = 0;
	int maxValue = 255;
	int currentMinThreshold = 0;
	int currentMaxThreshold = 255;
	int windowWidth = 256;
	int windowLevel = 128;

	bool showPlane = false;
	double planeOrigin[3] = {};
	double planeP1[3] = {};
	double planeP2[3] = {};
	bool checkMedian = false;
	int medianKernelX = 1;
	int medianKernelY = 1;
	int medianKernelZ = 1;
	int orientIndex = 0;
	bool orientChanged = false;
	int planeOrientation = 0;
	bool changePlaneInput = false;
	bool switchPlane = false;
	bool planeIsVisible = true;
	unsigned long planeObserverTag = 0;

	// Dirty flags keep inexpensive UI changes from rebuilding the complete
	// VTK pipeline. Each pipeline clears the flags after applying them.
	bool transformDirty = true;
	bool appearanceDirty = true;
	bool dataPipelineDirty = true;
	bool planeDirty = true;
	bool visibilityDirty = true;
	bool pipelineInitialized = false;
};

struct Settings
{
	bool oneFileLoaded = false;
	bool firstFileLoaded = false;
	int background_RGB1[3] = {};
	int background_RGB2[3] = {};
	bool showAxesTriad = false;
	bool showAxesBox = false;
	bool showOrientAxes = false;
	bool showScalarBar = false;
	double x_rot_cam = 0.0;
	double y_rot_cam = 0.0;
	double z_rot_cam = 0.0;
};
