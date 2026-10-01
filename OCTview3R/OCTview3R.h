#ifndef OCTVIEW3R_H
#define OCTVIEW3R_H

#include "documentModel.h"
#include "viewerData.h"
#include "opendata.h"
#include "openpoly.h"

#include <QList>
#include <QMainWindow>
#include <memory>

// Forward Qt class declarations
class Ui_OCTview3R;
class QAction;
class QCloseEvent;
class QComboBox;
class QGroupBox;
class OpenData;
class OpenPolyData;
class QLabel;
class QPushButton;
class QSpinBox;
class QThread;
class ViewerController;

// Forward VTK class declarations
class vtkImageData;
class vtkPolyData;
class vtkObject;
class vtkCamera;
class vtkTransform;

/**
 * @brief GUI-thread bridge between Designer controls, document state and rendering.
 *
 * Slots validate user input, edit only the selected document, mark affected
 * pipeline stages dirty, and request a refresh. Tab restoration blocks signals
 * so assigning controls cannot overwrite another document's values. Layout stays
 * in OCTview3R.ui; processing belongs in the focused pipeline classes.
 * See docs/developer-guide.md for the event flow and extension checklist.
 */
class OCTview3R : public QMainWindow
{
	Q_OBJECT

public:
	/// Create the Designer UI, scene controller, preferences and signal connections.
	OCTview3R();
	/// Wait for readers, detach scene resources, then destroy documents and the UI.
	~OCTview3R() override;

public slots:
	/// Snapshot validated dialog settings and start a volume-loading worker thread.
	virtual void slotProcessDataFile();
	/// Geometry equivalent of slotProcessDataFile(); no renderer calls run in the worker.
	virtual void slotProcessPolyFile();
	virtual void slotExit();

protected:
	void closeEvent(QCloseEvent* event) override;
	vtkCamera *cam;
	vtkSmartPointer<vtkTransform> camTrans;

signals:
	void signalLoadFileStarted(void);
	void signalLoadFileFinished(void);

protected slots:
	/// Consume the active document's dirty flags; camera reset is limited to first-load setup.
	void refreshViewer(void);

	// Document selection and queued worker results (all handlers execute on the GUI thread).
	/// Restore controls from a borrowed loaded model document with child signals blocked.
	void slotSetImageData(ImageData*);
	/// Select the model/tab index and update decorations without rebuilding dataset pipelines.
	void slotSetImageData(int);
	void slotOpenDataFileDialog(void);
	void slotOpenPolyFileDialog(void);
	void slotShowAbout(void);
	/// Adopt or release the worker's extra VTK reference, including rejected-result paths.
	void slotDataFileDialogClosed(
		vtkImageData*,
		unsigned int spacingInMillimetresMask,
		double displayScalarMinimum,
		double displayScalarMaximum);
	/// Geometry result with the same transfer-reference contract as slotDataFileDialogClosed().
	void slotPolyFileDialogClosed(vtkPolyData*);
	void slotDataLoadFailed(const QString&);
	void slotPolyLoadFailed(const QString&);

	// Threshold acceptance changes both data filtering and opacity/colour functions.
	void slotSetThreshold(void);
	void slotCheckMinThresholdSlider(int);
	void slotCheckMaxThresholdSlider(int);

	// Appearance controls; RGB window/level also changes derived image buffers.
	void slotSetColormap(QString);
	void slotAdjustColormap(bool);
	void slotAutoWindow(bool);
	void slotSmoothOpacity(bool);
	void slotSetBlendMode(int);
	void slotSetColorMode(int);
	void slotSetPolyMode(int);
	void slotInvertColormap(bool);
	void slotPickVolumeColor(void);
	void slotPickPolyColor(void);
	void slotSetObjectOpacity(int);
	void slotSetWindowWidth(int);
	void slotSetWindowLevel(int);
	void slotSetPolyGloss(int);
	void slotSetPointSize(int);

	// Per-document transforms, independent of camera rotation and other documents.
	void slotShowObject(bool);
	void slotRotX(double);
	void slotRotY(double);
	void slotRotZ(double);
	void slotShiftX(double);
	void slotShiftY(double);
	void slotShiftZ(double);

	// Plane controls: enabling the plane clips the volume; texture visibility is separate.
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
 
	// Scene decorations (do not modify source data or registration).
	void slotShowScalarBar(bool);
	void slotShowAxesTriad(bool);
	void slotShowOrientAxes(bool);
	void slotShowAxesBox(bool);

	//general slots
	/// Commit pending crop editors, convert calibrated units, round voxels and validate bounds.
	void slotApplyRanges(void);
	/// Mark editors pending only; do not run expensive crop filters on each keystroke.
	void slotRangesEdited(void);
	void slotResetRanges(void);
	void slotResetObjectTransform(void);
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
	void slotCloseTab(int index);
	void slotFitSelected(void);
	void slotFitAll(void);
	void slotParallelProjection(bool enabled);

private:
	static void onePlaneCallbackFunction(
		vtkObject *caller,
		unsigned long eventId,
		void *clientData,
		void *callData
	);
	// GUI-thread reentrancy guard, NOT a mutex or synchronization primitive.
	bool onePlaneCallbackMutex;
	void setupEnhancedUi();
	void setupObjectPanel();
	void setupNumericEditors();
	void setupCameraToolbar();
	void setupMetadataPanel();
	void updateColorModeControls();
	void updateMetadata();
	void updateRangePresentation();
	void setRangesPending(bool pending);
	void applyAutomaticWindowLevel();
	// These helpers set one flag only; they neither render nor propagate dependencies.
	void markTransformDirty();
	void markAppearanceDirty();
	void markDataPipelineDirty();
	void markPlaneDirty();
	void loadApplicationSettings();
	void saveApplicationSettings() const;
	void applyDarkTheme();
	void addDocumentTab(ImageData& data);

	QLabel *statusLabel;
	QLabel *metadataLabel = nullptr;
	QSpinBox *opacitySpinBox = nullptr;
	QSpinBox *glossSpinBox = nullptr;
	QSpinBox *minThresholdSpinBox = nullptr;
	QSpinBox *maxThresholdSpinBox = nullptr;
	QPushButton *resetRangesButton = nullptr;
	QPushButton *resetTransformButton = nullptr;
	QGroupBox *rangeGroupBox = nullptr;
	QAction *fitSelectedAction = nullptr;
	QAction *fitAllAction = nullptr;
	QAction *parallelProjectionAction = nullptr;

	// Designer form and GUI-owned dialogs.
	Ui_OCTview3R *ui;
	OpenData *openData;
	OpenPoly *openPoly;

	// Scene-wide state and document ownership. activeImageData is borrowed from
	// documentModel and must be replaced/cleared before its document is destroyed.
	Settings settings;
	ImageData* activeImageData = nullptr;
	DocumentModel documentModel;
	QList<QThread*> loadingThreads;
	std::unique_ptr<ViewerController> viewerController;

public:
	/// Bind the controller to the Designer QVTK widget once during GUI initialization.
	void initializeVTKPipeline();
};

#endif // OCTVIEW3R_H
