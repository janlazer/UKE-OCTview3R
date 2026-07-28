#pragma once

#include "polyPipeline.h"
#include "volumePipeline.h"

#include <vtkSmartPointer.h>

class vtkAxesActor;
class vtkCubeAxesActor;
class vtkOrientationMarkerWidget;
class vtkRenderWindow;
class vtkRenderWindowInteractor;
class vtkRenderer;
class vtkScalarBarActor;
class vtkScalarBarWidget;

class DocumentModel;
struct ImageData;
struct Settings;

class ViewerController final
{
public:
	ViewerController();
	~ViewerController();

	ViewerController(const ViewerController&) = delete;
	ViewerController& operator=(const ViewerController&) = delete;

	void initialize(vtkRenderWindow* renderWindow, const Settings& settings);
	void refresh(
		const DocumentModel& documents,
		ImageData* activeDocument,
		const Settings& settings);
	void refreshAll(
		const DocumentModel& documents,
		ImageData* activeDocument,
		const Settings& settings);
	void refreshDecorations(
		const DocumentModel& documents,
		ImageData* activeDocument,
		const Settings& settings);
	void detach(ImageData& document);
	void clear(const DocumentModel& documents);

	void setBackground1(int red, int green, int blue);
	void setBackground2(int red, int green, int blue);
	void render();

	vtkRenderer* renderer() const;
	vtkRenderWindow* renderWindow() const;
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
	vtkSmartPointer<vtkOrientationMarkerWidget> m_orientationWidget;
	vtkRenderWindow* m_renderWindow = nullptr;
	vtkRenderWindowInteractor* m_interactor = nullptr;
	VolumePipeline m_volumePipeline;
	PolyPipeline m_polyPipeline;
};
