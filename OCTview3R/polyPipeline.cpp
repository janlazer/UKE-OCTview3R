#include "polyPipeline.h"

#include "viewerData.h"

#include <vtkActor.h>
#include <vtkBox.h>
#include <vtkClipPolyData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

#include <cmath>

void PolyPipeline::update(
	ImageData& data,
	const Settings& settings,
	vtkRenderer* renderer) const
{
	data.transform->Identity();
	data.transform->PostMultiply();
	data.transform->Scale(settings.x_fac, settings.y_fac, settings.z_fac);
	data.transform->RotateX(data.rot[0]);
	data.transform->RotateY(data.rot[1]);
	data.transform->RotateZ(data.rot[2]);
	data.transform->Translate(data.shift);

	bool rangeIsRestricted = false;
	for (int i = 0; i < 6; ++i)
	{
		if (std::abs(data.VOI[i] - data.sourceVOI[i]) > 1e-9)
		{
			rangeIsRestricted = true;
			break;
		}
	}
	if (rangeIsRestricted)
	{
		data.polyClipBox->SetBounds(data.VOI);
		data.polyClipper->SetInputData(data.poly);
		data.polyClipper->SetClipFunction(data.polyClipBox);
		data.polyClipper->InsideOutOn();
		data.polyClipper->GenerateClippedOutputOff();
		data.polyMapper->SetInputConnection(data.polyClipper->GetOutputPort());
	}
	else
	{
		data.polyMapper->SetInputData(data.poly);
	}
	data.polyActor->SetMapper(data.polyMapper);
	data.polyActor->SetScale(1.0, 1.0, 1.0);
	data.polyActor->SetUserTransform(data.transform);
	// Keep the actor in one render pass across the full opacity range. Without
	// this, VTK switches from translucent to opaque rendering at exactly 1.0.
	data.polyActor->ForceTranslucentOn();

	vtkProperty* property = data.polyActor->GetProperty();
	switch (data.polyMode)
	{
	case 1:
		property->SetRepresentationToWireframe();
		break;
	case 2:
		property->SetRepresentationToSurface();
		break;
	default:
		property->SetRepresentationToPoints();
		property->SetPointSize(data.pointSize);
		break;
	}

	property->SetLighting(true);
	property->SetInterpolationToPhong();
	property->SetAmbient(0.2);
	property->SetDiffuse(0.8);
	property->SetSpecular(data.polyGloss);
	property->SetSpecularPower(20.0);
	property->SetOpacity(data.objectOpacity);
	property->SetColor(
		data.polyColor.redF(),
		data.polyColor.greenF(),
		data.polyColor.blueF());
	if (!renderer->HasViewProp(data.polyActor))
		renderer->AddActor(data.polyActor);
	data.polyActor->SetVisibility(data.showObject);
}
