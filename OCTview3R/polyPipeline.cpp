#include "polyPipeline.h"

#include "viewerData.h"

#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

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

	data.polyMapper->SetInputData(data.poly);
	data.polyActor->SetMapper(data.polyMapper);
	data.polyActor->SetScale(1.0, 1.0, 1.0);
	data.polyActor->SetUserTransform(data.transform);

	switch (data.polyMode)
	{
	case 1:
		data.polyActor->GetProperty()->SetRepresentationToWireframe();
		break;
	case 2:
		data.polyActor->GetProperty()->SetRepresentationToSurface();
		break;
	default:
		data.polyActor->GetProperty()->SetRepresentationToPoints();
		data.polyActor->GetProperty()->SetPointSize(data.pointSize);
		break;
	}

	data.polyActor->GetProperty()->SetOpacity(data.objectOpacity);
	data.polyActor->GetProperty()->SetColor(
		data.polyColor.redF(),
		data.polyColor.greenF(),
		data.polyColor.blueF());
	if (!renderer->HasViewProp(data.polyActor))
		renderer->AddActor(data.polyActor);
	data.polyActor->SetVisibility(data.showObject);
}
