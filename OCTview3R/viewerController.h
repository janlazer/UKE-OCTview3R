#pragma once

#include "polyPipeline.h"
#include "volumePipeline.h"

#include <vtkSmartPointer.h>

class vtkAxesActor;
class vtkCubeAxesActor;
class vtkLight;
class vtkOrientationMarkerWidget;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkScalarBarActor;
class vtkScalarBarWidget;

class DocumentModel;
struct ImageData;
struct Settings;

/**
 * @brief GUI-thread coordinator for one shared scene and its decorations.
 *
 * Owns renderer/decoration resources, not datasets or the Qt render window.
 * The render window and its interactor must remain valid while attached widgets
 * are in use, including controller teardown. Dataset pipelines retain their VTK
 * resources in ImageData. Active-tab selection controls editing/annotations,
 * not front-to-back occlusion. See docs/architecture.md for scene conventions.
 */
class ViewerController final
{
public:
	ViewerController();
	~ViewerController();

	ViewerController(const ViewerController&) = delete;
	ViewerController& operator=(const ViewerController&) = delete;

	/// Attach once to an initialized Qt/VTK window; missing window/interactor skips setup.
	void initialize(vtkRenderWindow* renderWindow, const Settings& settings);
	/** Update only activeDocument's dirty pipeline, then decorations and the frame.
	 * @param activeDocument Borrowed loaded document, or nullptr for an empty scene.
	 * @param resetCamera Explicit camera fit; ordinary edits must leave this false.
	 */
	void refresh(
		const DocumentModel& documents,
		ImageData* activeDocument,
		const Settings& settings,
		bool resetCamera = false);
	/// Refresh annotations and render without updating any dataset pipeline.
	void refreshDecorations(
		const DocumentModel& documents,
		ImageData* activeDocument,
		const Settings& settings);
	/// Disable plane observers/interactor and remove props before destroying the document.
	void detach(ImageData& document);
	/// Detach all documents and hide decorations; does not delete model-owned data.
	void clear(const DocumentModel& documents);

	/// Set the first background endpoint from 0..255 RGB; update annotations, not the frame.
	void setBackground1(int red, int green, int blue);
	/// Set the second background endpoint from 0..255 RGB; update annotations, not the frame.
	void setBackground2(int red, int green, int blue);
	/// Select light/dark annotation colours from average background luminance.
	void updateAnnotationColor();
	/// Fit and render using the document's current transformed actor/volume bounds.
	void fitToDocument(const ImageData& document);
	/// Fit and render all visible scene props.
	void fitAll();
	/// Change projection and render without resetting camera orientation.
	void setParallelProjection(bool enabled);
	bool parallelProjection() const;
	/// Render on the GUI thread; no-op until a render window has been assigned.
	void render();

	/// Borrow the owned renderer; never Delete() this returned pointer.
	vtkRenderer* renderer() const;
	/// Borrow the Qt-owned window, or nullptr before initialization.
	vtkRenderWindow* renderWindow() const;
	/// Borrow the window's interactor, or nullptr if initialization could not attach it.
	vtkRenderWindowInteractor* interactor() const;

private:
	void updateDocument(ImageData& document, const Settings& settings);
	void finishRefresh(
		const DocumentModel& documents,
		ImageData* activeDocument,
		const Settings& settings);
	void updateAxes(ImageData& activeDocument, const Settings& settings);
	void updateScalarBar(ImageData& activeDocument, const Settings& settings);
	void updateOrientationMarker(const Settings& settings);
	void hideDecorations();

	vtkSmartPointer<vtkRenderer> m_renderer;
	vtkSmartPointer<vtkScalarBarActor> m_scalarBarActor;
	vtkSmartPointer<vtkScalarBarWidget> m_scalarBarWidget;
	vtkSmartPointer<vtkCubeAxesActor> m_axes;
	vtkSmartPointer<vtkAxesActor> m_axesActor;
	vtkSmartPointer<vtkLight> m_headlight;
	vtkSmartPointer<vtkOrientationMarkerWidget> m_orientationWidget;
	vtkRenderWindow* m_renderWindow = nullptr;
	vtkRenderWindowInteractor* m_interactor = nullptr;
	bool m_scalarBarInitialized = false;
	VolumePipeline m_volumePipeline;
	PolyPipeline m_polyPipeline;
};
