#pragma once

#include "dataTypes.h"

#include <QColor>
#include <QString>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkColorTransferFunction;
class vtkExtractVOI;
class vtkImageData;
class vtkImageLuminance;
class vtkImageMapToColors;
class vtkImageMedian3D;
class vtkImagePlaneWidget;
class vtkImageThreshold;
class vtkImplicitPlaneWidget;
class vtkPiecewiseFunction;
class vtkPlane;
class vtkPlaneCollection;
class vtkPolyData;
class vtkPolyDataMapper;
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
	vtkSmartPointer<vtkVolume> volume;
	vtkSmartPointer<vtkActor> actor;
	vtkSmartPointer<vtkTransform> transform;
	vtkSmartPointer<vtkImageLuminance> luminance;
	vtkSmartPointer<vtkSmartVolumeMapper> volumeMapper;
	vtkSmartPointer<vtkPolyDataMapper> polyMapper;
	vtkSmartPointer<vtkActor> polyActor;
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
	double rot[3] = {};
	double shift[3] = {};
	double spacing[3] = { 1.0, 1.0, 1.0 };
	double objectOpacity = 0.5;
	bool showObject = false;

	QString colormapName = "Greyscale";
	bool adjustColormap = true;
	bool invertColormap = false;
	int blendMode = 0;
	int polyMode = 0;
	QColor polyColor = QColor(255, 0, 0);
	QColor volumeColor = QColor(255, 0, 0);
	int pointSize = 1;

	bool changedThreshold = false;
	int minValue = 0;
	int maxValue = 255;
	int currentMinThreshold = 0;
	int currentMaxThreshold = 255;

	bool initPlane = true;
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
};

struct Settings
{
	int activeIndex = 0;
	int numMaxIndex = 0;
	bool oneFileLoaded = false;
	bool firstFileLoaded = false;
	int background_RGB1[3] = {};
	int background_RGB2[3] = {};
	bool showAxesTriad = false;
	bool showAxesBox = false;
	bool showOrientAxes = false;
	bool showScalarBar = false;
	double x_fac = 1.0;
	double y_fac = 1.0;
	double z_fac = 1.0;
	double x_rot_cam = 0.0;
	double y_rot_cam = 0.0;
	double z_rot_cam = 0.0;
};
