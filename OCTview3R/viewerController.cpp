#include "viewerController.h"

#include "documentModel.h"
#include "viewerData.h"

#include <vtkAxesActor.h>
#include <vtkColorTransferFunction.h>
#include <vtkCubeAxesActor.h>
#include <vtkImagePlaneWidget.h>
#include <vtkLight.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarWidget.h>
#include <vtkVolume.h>

ViewerController::ViewerController()
	: m_renderer(vtkSmartPointer<vtkRenderer>::New()),
	  m_scalarBarActor(vtkSmartPointer<vtkScalarBarActor>::New()),
	  m_scalarBarWidget(vtkSmartPointer<vtkScalarBarWidget>::New()),
	  m_axes(vtkSmartPointer<vtkCubeAxesActor>::New()),
	  m_axesActor(vtkSmartPointer<vtkAxesActor>::New()),
	  m_headlight(vtkSmartPointer<vtkLight>::New()),
	  m_orientationWidget(vtkSmartPointer<vtkOrientationMarkerWidget>::New())
{
}

ViewerController::~ViewerController()
{
	if (m_scalarBarWidget)
		m_scalarBarWidget->Off();
	if (m_orientationWidget)
		m_orientationWidget->SetEnabled(0);
}

void ViewerController::initialize(
	vtkRenderWindow* renderWindow,
	const Settings& settings)
{
	m_renderWindow = renderWindow;
	m_interactor = renderWindow != nullptr ? renderWindow->GetInteractor() : nullptr;
	if (m_renderWindow == nullptr || m_interactor == nullptr)
		return;

	m_renderWindow->AddRenderer(m_renderer);
	m_renderWindow->SetAlphaBitPlanes(1);
	m_renderWindow->SetMultiSamples(0);
	m_interactor->SetDesiredUpdateRate(10.0);
	m_renderer->AutomaticLightCreationOff();
	m_renderer->RemoveAllLights();
	m_renderer->TwoSidedLightingOn();
	m_headlight->SetLightTypeToHeadlight();
	m_headlight->SetColor(1.0, 1.0, 1.0);
	m_headlight->SetIntensity(1.0);
	m_renderer->AddLight(m_headlight);
	m_renderer->SetUseDepthPeeling(1);
	m_renderer->SetMaximumNumberOfPeels(100);
	m_renderer->SetOcclusionRatio(0.1);
	m_renderer->GradientBackgroundOn();
	setBackground1(
		settings.background_RGB1[0],
		settings.background_RGB1[1],
		settings.background_RGB1[2]);
	setBackground2(
		settings.background_RGB2[0],
		settings.background_RGB2[1],
		settings.background_RGB2[2]);
}

void ViewerController::refresh(
	const DocumentModel& documents,
	ImageData* activeDocument,
	const Settings& settings,
	bool resetCamera)
{
	if (activeDocument != nullptr && activeDocument->fileLoaded)
		updateDocument(*activeDocument, settings);
	if (resetCamera)
		m_renderer->ResetCamera();
	finishRefresh(documents, activeDocument, settings);
}

void ViewerController::refreshAll(
	const DocumentModel& documents,
	ImageData* activeDocument,
	const Settings& settings)
{
	for (const DocumentModel::Document& document : documents.documents())
	{
		if (!document || !document->fileLoaded)
			continue;
		updateDocument(*document, settings);
	}
	finishRefresh(documents, activeDocument, settings);
}

void ViewerController::refreshDecorations(
	const DocumentModel& documents,
	ImageData* activeDocument,
	const Settings& settings)
{
	finishRefresh(documents, activeDocument, settings);
}

void ViewerController::finishRefresh(
	const DocumentModel& documents,
	ImageData* activeDocument,
	const Settings& settings)
{
	if (activeDocument == nullptr || documents.empty())
	{
		hideDecorations();
		render();
		return;
	}

	updateAxes(*activeDocument, settings);
	updateScalarBar(*activeDocument, settings);
	updateOrientationMarker(settings);
	m_renderer->ResetCameraClippingRange();
	render();
}

void ViewerController::updateDocument(
	ImageData& document,
	const Settings& settings)
{
	if (document.isVolume)
		m_volumePipeline.update(document, settings, m_renderer, m_interactor);
	else if (document.isPolyData)
		m_polyPipeline.update(document, settings, m_renderer);
}

void ViewerController::detach(ImageData& document)
{
	if (document.planeObserverTag != 0)
	{
		document.planeWidget->RemoveObserver(document.planeObserverTag);
		document.planeObserverTag = 0;
	}
	document.planeWidget->Off();
	document.planeWidget->SetInteractor(nullptr);
	m_renderer->RemoveVolume(document.volume);
	m_renderer->RemoveActor(document.polyActor);
}

void ViewerController::clear(const DocumentModel& documents)
{
	for (const DocumentModel::Document& document : documents.documents())
	{
		if (document)
			detach(*document);
	}
	hideDecorations();
	render();
}

void ViewerController::setBackground1(int red, int green, int blue)
{
	m_renderer->SetBackground(
		static_cast<double>(red) / 255.0,
		static_cast<double>(green) / 255.0,
		static_cast<double>(blue) / 255.0);
}

void ViewerController::setBackground2(int red, int green, int blue)
{
	m_renderer->SetBackground2(
		static_cast<double>(red) / 255.0,
		static_cast<double>(green) / 255.0,
		static_cast<double>(blue) / 255.0);
}

void ViewerController::render()
{
	if (m_renderWindow)
		m_renderWindow->Render();
}

vtkRenderer* ViewerController::renderer() const
{
	return m_renderer;
}

vtkRenderWindow* ViewerController::renderWindow() const
{
	return m_renderWindow;
}

vtkRenderWindowInteractor* ViewerController::interactor() const
{
	return m_interactor;
}

void ViewerController::updateAxes(
	ImageData& activeDocument,
	const Settings& settings)
{
	if (!settings.showAxesTriad && !settings.showAxesBox)
	{
		m_renderer->RemoveActor(m_axes);
		return;
	}

	const bool showLabels = settings.showAxesTriad;
	m_axes->SetXAxisLabelVisibility(showLabels);
	m_axes->SetYAxisLabelVisibility(showLabels);
	m_axes->SetZAxisLabelVisibility(showLabels);
	m_axes->SetXAxisMinorTickVisibility(showLabels);
	m_axes->SetYAxisMinorTickVisibility(showLabels);
	m_axes->SetZAxisMinorTickVisibility(showLabels);
	m_axes->SetXAxisTickVisibility(showLabels);
	m_axes->SetYAxisTickVisibility(showLabels);
	m_axes->SetZAxisTickVisibility(showLabels);
	m_axes->SetXLabelFormat("%6.1f");
	m_axes->SetYLabelFormat("%6.1f");
	m_axes->SetZLabelFormat("%6.1f");
	if (showLabels)
	{
		m_axes->SetScreenSize(12.0);
		m_axes->SetFlyModeToOuterEdges();
		m_axes->SetCornerOffset(0.0);
	}
	else
	{
		m_axes->SetFlyModeToStaticEdges();
	}

	if (activeDocument.isVolume)
		m_axes->SetBounds(activeDocument.volume->GetBounds());
	else if (activeDocument.isPolyData)
		m_axes->SetBounds(activeDocument.polyActor->GetBounds());
	m_axes->SetCamera(m_renderer->GetActiveCamera());
	m_axes->SetRebuildAxes(true);
	if (!m_renderer->HasViewProp(m_axes))
		m_renderer->AddActor(m_axes);
}

void ViewerController::updateScalarBar(
	ImageData& activeDocument,
	const Settings& settings)
{
	if (!settings.showScalarBar || !activeDocument.isVolume)
	{
		m_scalarBarWidget->Off();
		m_renderer->RemoveActor(m_scalarBarActor);
		return;
	}

	m_scalarBarActor->SetLookupTable(activeDocument.colorFun);
	m_scalarBarActor->SetTitle("Intensity");
	m_scalarBarActor->SetMaximumWidthInPixels(100);
	m_scalarBarActor->SetMaximumHeightInPixels(700);
	m_scalarBarWidget->SetInteractor(m_interactor);
	m_scalarBarWidget->SetScalarBarActor(m_scalarBarActor);
	m_scalarBarWidget->On();
	if (!m_renderer->HasViewProp(m_scalarBarActor))
		m_renderer->AddActor(m_scalarBarActor);
}

void ViewerController::updateOrientationMarker(const Settings& settings)
{
	m_orientationWidget->SetOrientationMarker(m_axesActor);
	m_orientationWidget->SetInteractor(m_interactor);
	m_orientationWidget->SetViewport(0.0, 0.0, 0.3, 0.3);
	m_orientationWidget->SetEnabled(settings.showOrientAxes ? 1 : 0);
}

void ViewerController::hideDecorations()
{
	m_renderer->RemoveActor(m_axes);
	m_scalarBarWidget->Off();
	m_renderer->RemoveActor(m_scalarBarActor);
	m_orientationWidget->SetEnabled(0);
}
