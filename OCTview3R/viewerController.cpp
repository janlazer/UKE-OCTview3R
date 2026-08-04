#include "viewerController.h"

#include "documentModel.h"
#include "viewerData.h"

#include <vtkAxesActor.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkCubeAxesActor.h>
#include <vtkImagePlaneWidget.h>
#include <vtkLight.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkScalarBarActor.h>
#include <vtkScalarBarRepresentation.h>
#include <vtkScalarBarWidget.h>
#include <vtkTextProperty.h>
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
	// PolyData is deliberately kept in the translucent pass at every opacity.
	// Include volumes in the same depth-peeling pass so front-to-back geometry,
	// rather than prop insertion order or the active tab, controls occlusion.
	m_renderer->SetUseDepthPeelingForVolumes(true);
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
	updateAnnotationColor();
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
	updateAnnotationColor();
}

void ViewerController::setBackground2(int red, int green, int blue)
{
	m_renderer->SetBackground2(
		static_cast<double>(red) / 255.0,
		static_cast<double>(green) / 255.0,
		static_cast<double>(blue) / 255.0);
	updateAnnotationColor();
}

void ViewerController::updateAnnotationColor()
{
	const double* first = m_renderer->GetBackground();
	const double* second = m_renderer->GetBackground2();
	const double luminance =
		0.2126 * (first[0] + second[0]) * 0.5 +
		0.7152 * (first[1] + second[1]) * 0.5 +
		0.0722 * (first[2] + second[2]) * 0.5;
	const double color = luminance < 0.5 ? 1.0 : 0.08;

	for (int axis = 0; axis < 3; ++axis)
	{
		m_axes->GetTitleTextProperty(axis)->SetColor(color, color, color);
		m_axes->GetLabelTextProperty(axis)->SetColor(color, color, color);
	}
	m_axes->GetXAxesLinesProperty()->SetColor(color, color, color);
	m_axes->GetYAxesLinesProperty()->SetColor(color, color, color);
	m_axes->GetZAxesLinesProperty()->SetColor(color, color, color);
	m_scalarBarActor->GetTitleTextProperty()->SetColor(color, color, color);
	m_scalarBarActor->GetLabelTextProperty()->SetColor(color, color, color);
	m_scalarBarActor->GetAnnotationTextProperty()->SetColor(color, color, color);
}

void ViewerController::fitToDocument(const ImageData& document)
{
	double bounds[6] = {};
	if (document.isVolume)
		document.volume->GetBounds(bounds);
	else if (document.isPolyData)
		document.polyActor->GetBounds(bounds);
	else
		return;
	m_renderer->ResetCamera(bounds);
	m_renderer->ResetCameraClippingRange();
	render();
}

void ViewerController::fitAll()
{
	m_renderer->ResetCamera();
	m_renderer->ResetCameraClippingRange();
	render();
}

void ViewerController::setParallelProjection(bool enabled)
{
	m_renderer->GetActiveCamera()->SetParallelProjection(enabled ? 1 : 0);
	m_renderer->ResetCameraClippingRange();
	render();
}

bool ViewerController::parallelProjection() const
{
	return m_renderer->GetActiveCamera()->GetParallelProjection() != 0;
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

	double bounds[6] = {};
	if (activeDocument.isVolume)
		activeDocument.volume->GetBounds(bounds);
	else if (activeDocument.isPolyData)
		activeDocument.polyActor->GetBounds(bounds);
	else
		return;

	double displayedRange[6] = {};
	for (int axis = 0; axis < 3; ++axis)
	{
		const bool showMicrometres = activeDocument.isVolume &&
			(activeDocument.spacingInMillimetresMask & (1U << axis)) != 0;
		const double factor = showMicrometres ? 1000.0 : 1.0;
		displayedRange[2 * axis] = factor * bounds[2 * axis];
		displayedRange[2 * axis + 1] = factor * bounds[2 * axis + 1];
	}

	// Keep the physical bounds in the renderer's internal millimetre space,
	// but label calibrated OCT axes in micrometres.
	m_axes->SetBounds(bounds);
	m_axes->SetXAxisRange(displayedRange[0], displayedRange[1]);
	m_axes->SetYAxisRange(displayedRange[2], displayedRange[3]);
	m_axes->SetZAxisRange(displayedRange[4], displayedRange[5]);
	const char* micrometre = "\xC2\xB5m";
	m_axes->SetXTitle("X");
	m_axes->SetYTitle("Y");
	m_axes->SetZTitle("Z");
	m_axes->SetXUnits(activeDocument.isVolume &&
		(activeDocument.spacingInMillimetresMask & (1U << 0)) != 0
		? micrometre : "");
	m_axes->SetYUnits(activeDocument.isVolume &&
		(activeDocument.spacingInMillimetresMask & (1U << 1)) != 0
		? micrometre : "");
	m_axes->SetZUnits(activeDocument.isVolume &&
		(activeDocument.spacingInMillimetresMask & (1U << 2)) != 0
		? micrometre : "");
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
	// The previous 100-pixel cap made horizontal widget resizing appear
	// ineffective. Keep only a generous safety limit and let the interactive
	// representation determine the actual on-screen dimensions.
	m_scalarBarActor->SetMaximumWidthInPixels(10000);
	m_scalarBarActor->SetMaximumHeightInPixels(10000);
	m_scalarBarWidget->SetInteractor(m_interactor);
	m_scalarBarWidget->SetScalarBarActor(m_scalarBarActor);
	m_scalarBarWidget->RepositionableOn();
	m_scalarBarWidget->CreateDefaultRepresentation();
	if (!m_scalarBarInitialized)
	{
		vtkScalarBarRepresentation* representation =
			m_scalarBarWidget->GetScalarBarRepresentation();
		if (representation != nullptr)
		{
			// Start at approximately half the former default size. Turning off
			// proportional resizing allows the left/right edges to change only
			// the width while the user drags them.
			representation->ProportionalResizeOff();
			representation->SetPosition(0.90, 0.30);
			representation->SetPosition2(0.08, 0.40);
			representation->SetShowBorderToActive();
			m_scalarBarInitialized = true;
		}
	}
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
