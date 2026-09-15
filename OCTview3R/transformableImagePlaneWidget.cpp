#include "transformableImagePlaneWidget.h"

#include <vtkAbstractPropPicker.h>
#include <vtkActor.h>
#include <vtkAssemblyNode.h>
#include <vtkAssemblyPath.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCollection.h>
#include <vtkCommand.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkImageReslice.h>
#include <vtkLinearTransform.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPlaneSource.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkProp.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#include <algorithm>

vtkStandardNewMacro(TransformableImagePlaneWidget);

void TransformableImagePlaneWidget::SetDisplayTransform(
	vtkLinearTransform* transform)
{
	DisplayTransform = transform;
	// Transform display props together, leaving PlaneSource and Reslice in image
	// space. Applying the same transform to the input image would transform twice.
	PlaneOutlineActor->SetUserTransform(transform);
	TexturePlaneActor->SetUserTransform(transform);
	CursorActor->SetUserTransform(transform);
	MarginActor->SetUserTransform(transform);
}

void TransformableImagePlaneWidget::StartCursor()
{
	vtkImagePlaneWidget::StartCursor();
	if (State != vtkImagePlaneWidget::Cursoring)
		return;

	const int* position = Interactor->GetEventPosition();
	UpdateTransformedCursor(position[0], position[1]);
	ManageTextDisplay();
	Interactor->Render();
}

void TransformableImagePlaneWidget::StartSliceMotion()
{
	vtkImagePlaneWidget::StartSliceMotion();
	if (DisplayTransform == nullptr ||
		State == vtkImagePlaneWidget::Outside ||
		State == vtkImagePlaneWidget::Start)
	{
		return;
	}

	// The picker returns world coordinates, while base-class state/margin tests
	// use PlaneSource's local coordinates. Restore the world pick afterwards:
	// OnMouseMove needs it to recover display depth for the next mouse event.
	double worldPick[3] = {
		LastPickPosition[0],
		LastPickPosition[1],
		LastPickPosition[2]
	};
	DisplayTransform->GetLinearInverse()->TransformPoint(
		worldPick, LastPickPosition);
	AdjustState();
	UpdateMargins();
	LastPickPosition[0] = worldPick[0];
	LastPickPosition[1] = worldPick[1];
	LastPickPosition[2] = worldPick[2];
}

void TransformableImagePlaneWidget::OnMouseMove()
{
	if (State == vtkImagePlaneWidget::Outside ||
		State == vtkImagePlaneWidget::Start)
	{
		return;
	}

	const int x = Interactor->GetEventPosition()[0];
	const int y = Interactor->GetEventPosition()[1];
	vtkCamera* camera = CurrentRenderer->GetActiveCamera();
	if (camera == nullptr)
		return;

	double focalPoint[4] = {};
	double worldPick[4] = {};
	double previousWorldPick[4] = {};
	ComputeWorldToDisplay(
		LastPickPosition[0],
		LastPickPosition[1],
		LastPickPosition[2],
		focalPoint);
	const double depth = focalPoint[2];
	ComputeDisplayToWorld(
		static_cast<double>(Interactor->GetLastEventPosition()[0]),
		static_cast<double>(Interactor->GetLastEventPosition()[1]),
		depth,
		previousWorldPick);
	ComputeDisplayToWorld(
		static_cast<double>(x),
		static_cast<double>(y),
		depth,
		worldPick);

	// Project both mouse positions at the last picked depth, then undo the actor
	// transform before calling base-class motion helpers. This keeps dragging
	// consistent after rotation, translation and non-uniform dataset scaling.
	double previousLocalPick[3] = {
		previousWorldPick[0],
		previousWorldPick[1],
		previousWorldPick[2]
	};
	double localPick[3] = { worldPick[0], worldPick[1], worldPick[2] };
	vtkLinearTransform* inverse =
		DisplayTransform != nullptr
		? DisplayTransform->GetLinearInverse()
		: nullptr;
	if (inverse != nullptr)
	{
		inverse->TransformPoint(previousWorldPick, previousLocalPick);
		inverse->TransformPoint(worldPick, localPick);
	}

	if (State == vtkImagePlaneWidget::WindowLevelling)
	{
		WindowLevel(x, y);
		ManageTextDisplay();
	}
	else if (State == vtkImagePlaneWidget::Pushing)
	{
		Push(previousLocalPick, localPick);
		UpdatePlane();
		UpdateMargins();
		BuildRepresentation();
	}
	else if (State == vtkImagePlaneWidget::Spinning)
	{
		Spin(previousLocalPick, localPick);
		UpdatePlane();
		UpdateMargins();
		BuildRepresentation();
	}
	else if (State == vtkImagePlaneWidget::Rotating)
	{
		double worldViewNormal[3] = {};
		double localViewNormal[3] = {};
		camera->GetViewPlaneNormal(worldViewNormal);
		// Normals require TransformNormal (inverse-transpose semantics), not the
		// point/vector transform, especially when the dataset scale is anisotropic.
		if (inverse != nullptr)
			inverse->TransformNormal(worldViewNormal, localViewNormal);
		else
			std::copy(worldViewNormal, worldViewNormal + 3, localViewNormal);
		Rotate(previousLocalPick, localPick, localViewNormal);
		UpdatePlane();
		UpdateMargins();
		BuildRepresentation();
	}
	else if (State == vtkImagePlaneWidget::Scaling)
	{
		Scale(previousLocalPick, localPick, x, y);
		UpdatePlane();
		UpdateMargins();
		BuildRepresentation();
	}
	else if (State == vtkImagePlaneWidget::Moving)
	{
		Translate(previousLocalPick, localPick);
		UpdatePlane();
		UpdateMargins();
		BuildRepresentation();
	}
	else if (State == vtkImagePlaneWidget::Cursoring)
	{
		UpdateTransformedCursor(x, y);
		ManageTextDisplay();
	}

	// Consume the event so camera interaction does not also handle the same drag.
	// Preserve VTK's event contract: window/level emits its own event, other plane
	// changes emit InteractionEvent for the application's clipping refresh.
	EventCallbackCommand->SetAbortFlag(1);
	if (State == vtkImagePlaneWidget::WindowLevelling)
	{
		double windowLevel[2] = { CurrentWindow, CurrentLevel };
		InvokeEvent(vtkCommand::WindowLevelEvent, windowLevel);
	}
	else
	{
		InvokeEvent(vtkCommand::InteractionEvent, nullptr);
	}
	Interactor->Render();
}

void TransformableImagePlaneWidget::UpdateTransformedCursor(int x, int y)
{
	if (ImageData == nullptr)
		return;

	Reslice->GetInputAlgorithm()->Update();
	vtkAssemblyPath* path =
		GetAssemblyPath(x, y, 0.0, PlanePicker);
	CurrentImageValue = VTK_DOUBLE_MAX;

	bool found = false;
	if (path != nullptr)
	{
		vtkCollectionSimpleIterator iterator;
		path->InitTraversal(iterator);
		for (int index = 0; index < path->GetNumberOfItems() && !found; ++index)
		{
			vtkAssemblyNode* node = path->GetNextNode(iterator);
			found = node->GetViewProp() ==
				vtkProp::SafeDownCast(TexturePlaneActor);
		}
	}

	if (!found || path == nullptr)
	{
		CursorActor->VisibilityOff();
		return;
	}
	CursorActor->VisibilityOn();

	double worldPick[3] = {};
	double localPick[3] = {};
	// Hit-test the displayed actor in world space, sample the image in local
	// space. Crosshair geometry stays local; CursorActor applies the display transform.
	PlanePicker->GetPickPosition(worldPick);
	if (DisplayTransform != nullptr)
	{
		DisplayTransform->GetLinearInverse()->TransformPoint(
			worldPick, localPick);
	}
	else
	{
		std::copy(worldPick, worldPick + 3, localPick);
	}

	found = UseContinuousCursor
		? UpdateContinuousCursor(localPick) != 0
		: UpdateDiscreteCursor(localPick) != 0;
	if (!found)
	{
		CursorActor->VisibilityOff();
		return;
	}

	double origin[3] = {};
	PlaneSource->GetOrigin(origin);
	double relativePick[3] = {
		localPick[0] - origin[0],
		localPick[1] - origin[1],
		localPick[2] - origin[2]
	};
	double vector1[3] = {};
	double vector2[3] = {};
	GetVector1(vector1);
	GetVector2(vector2);
	const double fraction1 =
		vtkMath::Dot(relativePick, vector1) /
		vtkMath::Dot(vector1, vector1);
	const double fraction2 =
		vtkMath::Dot(relativePick, vector2) /
		vtkMath::Dot(vector2, vector2);

	double point1[3] = {};
	double point2[3] = {};
	PlaneSource->GetPoint1(point1);
	PlaneSource->GetPoint2(point2);
	double cursorPoints[4][3] = {};
	for (int axis = 0; axis < 3; ++axis)
	{
		cursorPoints[0][axis] = origin[axis] + fraction2 * vector2[axis];
		cursorPoints[1][axis] = point1[axis] + fraction2 * vector2[axis];
		cursorPoints[2][axis] = origin[axis] + fraction1 * vector1[axis];
		cursorPoints[3][axis] = point2[axis] + fraction1 * vector1[axis];
	}

	vtkPoints* points = CursorPolyData->GetPoints();
	for (int index = 0; index < 4; ++index)
		points->SetPoint(index, cursorPoints[index]);
	points->GetData()->Modified();
	CursorPolyData->Modified();
}
