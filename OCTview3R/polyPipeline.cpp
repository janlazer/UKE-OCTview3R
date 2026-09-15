#include "polyPipeline.h"

#include "viewerData.h"

#include <vtkActor.h>
#include <vtkBox.h>
#include <vtkClipPolyData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>

#include <cmath>

void PolyPipeline::update(
	ImageData& data,
	const Settings& settings,
	vtkRenderer* renderer) const
{
	Q_UNUSED(settings);
	const bool initialize = !data.pipelineInitialized;

	if (initialize || data.transformDirty)
	{
		data.transform->Identity();
		data.transform->PostMultiply();
		data.transform->Scale(data.scale);
		data.transform->RotateX(data.rot[0]);
		data.transform->RotateY(data.rot[1]);
		data.transform->RotateZ(data.rot[2]);
		data.transform->Translate(data.shift);
		data.polyActor->SetScale(1.0, 1.0, 1.0);
		data.polyActor->SetUserTransform(data.transform);
	}

	if (initialize || data.dataPipelineDirty)
	{
		// Clip the original mesh before its display transform. VOI contains native
		// floating-point mesh bounds; applying volume voxel rounding here is incorrect.
		bool rangeIsRestricted = false;
		for (int i = 0; i < 6; ++i)
		{
			if (std::abs(data.VOI[i] - data.sourceVOI[i]) > 1e-9)
			{
				rangeIsRestricted = true;
				break;
			}
		}

		const bool hasSurfaces =
			data.poly->GetNumberOfPolys() > 0 ||
			data.poly->GetNumberOfStrips() > 0;
		if (rangeIsRestricted)
		{
			data.polyClipBox->SetBounds(data.VOI);
			data.polyClipper->SetInputData(data.poly);
			data.polyClipper->SetClipFunction(data.polyClipBox);
			data.polyClipper->InsideOutOn();
			data.polyClipper->GenerateClippedOutputOff();
			if (hasSurfaces)
				data.polyNormals->SetInputConnection(
					data.polyClipper->GetOutputPort());
			else
				data.polyMapper->SetInputConnection(
					data.polyClipper->GetOutputPort());
		}
		else if (hasSurfaces)
		{
			data.polyNormals->SetInputData(data.poly);
		}
		else
		{
			data.polyMapper->SetInputData(data.poly);
		}

		if (hasSurfaces)
		{
			data.polyNormals->ComputePointNormalsOn();
			data.polyNormals->ComputeCellNormalsOff();
			data.polyNormals->ConsistencyOn();
			data.polyNormals->AutoOrientNormalsOff();
			data.polyNormals->SplittingOn();
			data.polyNormals->SetFeatureAngle(60.0);
			data.polyMapper->SetInputConnection(data.polyNormals->GetOutputPort());
		}
		data.polyMapper->ScalarVisibilityOff();
		data.polyActor->SetMapper(data.polyMapper);
	}

	// Keep the actor in one render pass across the full opacity range. Without
	// this, VTK switches from translucent to opaque rendering at exactly 1.0.
	data.polyActor->ForceTranslucentOn();

	if (initialize || data.appearanceDirty)
	{
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
	}
	if (!renderer->HasViewProp(data.polyActor))
		renderer->AddActor(data.polyActor);
	if (initialize || data.visibilityDirty)
		data.polyActor->SetVisibility(data.showObject);

	data.transformDirty = false;
	data.appearanceDirty = false;
	data.dataPipelineDirty = false;
	data.planeDirty = false;
	data.visibilityDirty = false;
	data.pipelineInitialized = true;
}
