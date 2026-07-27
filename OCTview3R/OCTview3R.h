#ifndef OCTVIEW3R_H
#define OCTVIEW3R_H

#include "viewerData.h"
#include "opendata.h"
#include "openpoly.h"

#include <QList>
#include <QMainWindow>

// Forward Qt class declarations
class Ui_OCTview3R;
class OpenData;
class OpenPolyData;
class QLabel;
class QThread;

// Forward VTK class declarations
class vtkImageData;
class vtkPolyData;
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
class vtkOrientationMarkerWidget;
class vtkAxesActor;

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
	vtkSmartPointer<vtkRenderer> renderer;
	vtkRenderWindow* renWin;
	vtkRenderWindowInteractor* iren;
	vtkSmartPointer<vtkScalarBarActor> scalarBarActor;
	vtkSmartPointer<vtkScalarBarWidget> scalarBarWidget;
	vtkSmartPointer<vtkCubeAxesActor> axes;
	vtkCamera *cam;
	vtkSmartPointer<vtkTransform> camTrans;
	vtkSmartPointer<vtkAxesActor> axesActor;
	vtkSmartPointer<vtkOrientationMarkerWidget> orientWidget;

signals:
	void signalLoadFileStarted(void);
	void signalLoadFileFinished(void);

protected slots:
	void connectVTKPipeline(void);

	//new file
	void slotSetImageData(ImageData*);
	void slotSetImageData(int);
	void slotOpenDataFileDialog(void);
	void slotOpenPolyFileDialog(void);
	void slotDataFileDialogClosed(vtkImageData*);
	void slotPolyFileDialogClosed(vtkPolyData*);
	void slotDataLoadFailed(const QString&);
	void slotPolyLoadFailed(const QString&);

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
	static void onePlaneCallbackFunction(
		vtkObject *caller,
		unsigned long eventId,
		void *clientData,
		void *callData
	);
	bool onePlaneCallbackMutex;

	QLabel *statusLabel;

	//designer form
	Ui_OCTview3R *ui;
	OpenData *openData;
	OpenPoly *openPoly;

	//parameter structures
	Settings settings;
	ImageData* activeImageData = nullptr;
	QList<ImageData*> imageDataList;
	QList<QThread*> loadingThreads;

public:
	//helper
	ImageData* getInitializedImageData();
	void initializeVTKPipeline();
};

#endif // OCTVIEW3R_H
