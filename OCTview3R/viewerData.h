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

/**
 * @brief State and VTK resources for one volume OR geometry document.
 *
 * Owned by DocumentModel, mutated on the GUI thread after import. Original image
 * or poly inputs are retained separately from derived display/filter outputs.
 * Most settings are intentionally per-document, not copied from the active tab
 * when another file is loaded. Changing a field alone does not refresh VTK: the
 * caller must set the corresponding dirty flags and ask ViewerController to refresh.
 */
struct ImageData
{
	// Reference-counted pipeline/scene resources; never manually Delete() these members.
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
	// Pair order: Xmin, Xmax, Ymin, Ymax, Zmin, Zmax.
	// Volumes: integer-valued voxel indices, with EXCLUSIVE upper bounds.
	// PolyData: continuous native-coordinate bounds, not voxel indices.
	// The GUI may show calibrated micrometres; those display values are not stored here.
	double VOI[6] = {};
	double sourceVOI[6] = {}; ///< Full uncropped bounds in the same convention as VOI.
	double rot[3] = {}; ///< Degrees, applied in X/Y/Z order after scaling.
	double shift[3] = {}; ///< Translation after rotation, in scene units (not GUI micrometres).
	double scale[3] = { 1.0, 1.0, 1.0 }; ///< Dimensionless, relative to input geometry/spacing.
	double spacing[3] = { 1.0, 1.0, 1.0 }; ///< Image spacing; mm/voxel only on calibrated axes.
	// Bits 0/1/2 identify X/Y/Z spacing known to be in mm. A clear bit is unknown
	// physical calibration, not permission to label a spacing value as millimetres.
	unsigned int spacingInMillimetresMask = 0;
	double objectOpacity = 1.0;
	double polyGloss = 0.1;
	bool showObject = false;

	QString colormapName = "Greyscale";
	bool adjustColormap = true; ///< Stretch palette across thresholds, independent of window/level.
	bool autoWindow = true; ///< Neutral full-source-range window; false uses manual width/level.
	bool smoothOpacity = true; ///< Scalar opacity ramp; not used by the binary RGB alpha mask.
	bool invertColormap = false;
	bool renderRgb = false; ///< Effective only for input with at least three components.
	int blendMode = 0; ///< 0: MaxIP, 1: composite, 2: additive, 3: MinIP.
	int polyMode = 0; ///< 0: points, 1: wireframe, 2: surface.
	QColor polyColor = QColor(255, 0, 0);
	QColor volumeColor = QColor(255, 0, 0);
	int pointSize = 1;

	// Intensity units, not spatial units. RGB uses a luminance-derived display range.
	int minValue = 0;
	int maxValue = 255;
	int currentMinThreshold = 0;
	int currentMaxThreshold = 255;
	int windowWidth = 256;
	int windowLevel = 128;

	bool showPlane = false; ///< Enable plane interaction AND volume clipping by that plane.
	// Cached plane corners in local image coordinates (including image spacing),
	// before the dataset display transform; not voxel indices or world-space picks.
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
	bool changePlaneInput = false; ///< Reconfigure input/orientation; planeDirty alone does not do this.
	bool switchPlane = false; ///< Reverse the retained clipping half-space.
	bool planeIsVisible = true; ///< Show texture/margins; hiding them does not disable clipping.
	unsigned long planeObserverTag = 0; ///< InteractionEvent observer handle; zero means none.

	// Dirty flags keep inexpensive UI changes from rebuilding the complete
	// VTK pipeline. Each pipeline clears the flags after applying them. These are
	// independent: e.g. threshold changes need both appearanceDirty and dataPipelineDirty.
	// Plane input reconnection additionally requires changePlaneInput; see the
	// dependency table in docs/developer-guide.md before adding a new control.
	bool transformDirty = true;
	bool appearanceDirty = true;
	bool dataPipelineDirty = true;
	bool planeDirty = true;
	bool visibilityDirty = true;
	bool pipelineInitialized = false;
};

/// Scene-wide preferences. Dataset appearance and spatial transforms belong to ImageData.
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
