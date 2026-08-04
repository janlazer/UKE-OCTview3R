#include "volumePipeline.h"

#include "transformableImagePlaneWidget.h"
#include "viewerData.h"

#include <vtkColorTransferFunction.h>
#include <vtkExtractVOI.h>
#include <vtkImageData.h>
#include <vtkImageLuminance.h>
#include <vtkImageMapToColors.h>
#include <vtkImageMedian3D.h>
#include <vtkImagePlaneWidget.h>
#include <vtkImageThreshold.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkProperty.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkTransform.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace
{
using Rgb = std::array<double, 3>;

double clamp01(double value)
{
	return std::max(0.0, std::min(1.0, value));
}

Rgb interpolate(const Rgb& first, const Rgb& second, double factor)
{
	return {
		first[0] + factor * (second[0] - first[0]),
		first[1] + factor * (second[1] - first[1]),
		first[2] + factor * (second[2] - first[2])
	};
}

Rgb rainbow(double value)
{
	static const std::array<double, 6> positions = {
		0.0, 0.125, 0.375, 0.625, 0.875, 1.0
	};
	static const std::array<Rgb, 6> colors = {{
		{ 0.0, 0.0, 0.5 },
		{ 0.0, 0.0, 1.0 },
		{ 0.0, 1.0, 1.0 },
		{ 1.0, 1.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 0.5, 0.0, 0.0 }
	}};

	const double t = clamp01(value);
	for (std::size_t i = 1; i < positions.size(); ++i)
	{
		if (t <= positions[i])
		{
			const double segment =
				(t - positions[i - 1]) / (positions[i] - positions[i - 1]);
			return interpolate(colors[i - 1], colors[i], segment);
		}
	}
	return colors.back();
}

Rgb blackBody(double value)
{
	const double t = clamp01(value);
	if (t <= 0.25)
		return { 4.0 * t, 0.0, 0.0 };
	if (t <= 0.75)
		return { 1.0, 2.0 * t - 0.5, 0.0 };
	return { 1.0, 1.0, 4.0 * t - 3.0 };
}

Rgb colorAt(const ImageData& data, double value)
{
	double t = data.invertColormap ? 1.0 - clamp01(value) : clamp01(value);
	const Rgb selected = {
		data.volumeColor.redF(),
		data.volumeColor.greenF(),
		data.volumeColor.blueF()
	};

	if (data.colormapName == "Rainbow")
		return rainbow(t);
	if (data.colormapName == "Black Body")
		return blackBody(t);
	if (data.colormapName == "White to")
		return interpolate({ 1.0, 1.0, 1.0 }, selected, t);
	if (data.colormapName == "Black to")
		return interpolate({ 0.0, 0.0, 0.0 }, selected, t);
	if (data.colormapName == "Color all to")
		return selected;
	return { t, t, t };
}

std::vector<double> gradientStops(const QString& name)
{
	if (name == "Rainbow")
		return { 0.0, 0.125, 0.375, 0.625, 0.875, 1.0 };
	if (name == "Black Body")
		return { 0.0, 0.25, 0.75, 1.0 };
	return { 0.0, 1.0 };
}

void addColorPoint(ImageData& data, double scalar, double normalizedPosition)
{
	const Rgb color = colorAt(data, normalizedPosition);
	data.colorFun->AddRGBPoint(scalar, color[0], color[1], color[2]);
}

double scalarStep(const ImageData& data)
{
	if (data.image == nullptr)
		return 1.0;
	const int scalarType = data.image->GetScalarType();
	if (scalarType != VTK_FLOAT && scalarType != VTK_DOUBLE)
		return 1.0;
	const double range = std::abs(
		static_cast<double>(data.maxValue - data.minValue));
	return std::max(1e-12, range * 1e-6);
}

void configureOpacity(
	ImageData& data,
	double lower,
	double upper)
{
	data.opacityFun->RemoveAllPoints();
	data.opacityFun->ClampingOff();
	const double opacity = clamp01(data.objectOpacity);
	const double step = scalarStep(data);

	// Zero is the transparent black mask. Values surviving the threshold
	// receive a constant opacity instead of the previous intensity-dependent
	// ramp, which made mid-grey voxels look unintentionally translucent.
	data.opacityFun->AddPoint(0.0, 0.0);
	if (lower < 0.0)
	{
		data.opacityFun->AddPoint(lower, opacity);
		const double negativeEnd = std::min(upper, -step);
		if (negativeEnd > lower)
			data.opacityFun->AddPoint(negativeEnd, opacity);
	}
	if (upper > 0.0)
	{
		const double positiveStart = std::max(lower, step);
		if (positiveStart <= upper)
		{
			data.opacityFun->AddPoint(positiveStart, opacity);
			if (upper > positiveStart)
				data.opacityFun->AddPoint(upper, opacity);
		}
	}
}
}

void VolumePipeline::update(
	ImageData& data,
	const Settings& settings,
	vtkRenderer* renderer,
	vtkRenderWindowInteractor* interactor) const
{
	Q_UNUSED(settings);
	const bool initialize = !data.pipelineInitialized;
	const bool transformChanged = initialize || data.transformDirty;
	const bool appearanceChanged = initialize || data.appearanceDirty;
	const bool inputChanged = initialize || data.dataPipelineDirty;
	const bool planeChanged = initialize || data.planeDirty;
	const bool visibilityChanged = initialize || data.visibilityDirty;

	if (transformChanged)
	{
		data.transform->Identity();
		data.transform->PostMultiply();
		data.transform->Scale(data.scale);
		data.transform->RotateX(data.rot[0]);
		data.transform->RotateY(data.rot[1]);
		data.transform->RotateZ(data.rot[2]);
		data.transform->Translate(data.shift);
	}
	if (appearanceChanged)
		updateTransferFunctions(data);
	if (inputChanged)
		updateImageFilters(data);
	if (transformChanged || inputChanged || planeChanged)
		updatePlane(data, interactor);
	if (transformChanged || appearanceChanged || inputChanged ||
		planeChanged || visibilityChanged)
		updateVolume(data, renderer);

	data.transformDirty = false;
	data.appearanceDirty = false;
	data.dataPipelineDirty = false;
	data.planeDirty = false;
	data.visibilityDirty = false;
	data.pipelineInitialized = true;
}

void VolumePipeline::updateTransferFunctions(ImageData& data) const
{
	data.colorFun->RemoveAllPoints();
	data.colorFun->SetColorSpaceToRGB();

	const double thresholdLower = data.currentMinThreshold;
	const double thresholdUpper =
		data.currentMaxThreshold > data.currentMinThreshold
		? data.currentMaxThreshold
		: data.currentMinThreshold + 1.0;
	const double window = std::max(1, data.windowWidth);
	const double center = static_cast<double>(data.windowLevel) - 0.5;
	const double windowSpan = std::max(1.0, window - 1.0);
	auto windowPosition = [center, windowSpan](double scalar) {
		return clamp01((scalar - center) / windowSpan + 0.5);
	};
	auto scalarAtWindowPosition = [center, windowSpan](double position) {
		return center + (position - 0.5) * windowSpan;
	};
	const std::vector<double> stops = gradientStops(data.colormapName);

	// Window/level controls the colour mapping only; thresholding and the
	// zero-opacity mask remain independent. Endpoints at the threshold limits
	// keep values outside the window saturated instead of turning them black.
	addColorPoint(data, thresholdLower, windowPosition(thresholdLower));
	for (double stop : stops)
	{
		const double scalar = scalarAtWindowPosition(stop);
		if (scalar > thresholdLower && scalar < thresholdUpper)
			addColorPoint(data, scalar, stop);
	}
	addColorPoint(data, thresholdUpper, windowPosition(thresholdUpper));
	data.colorFun->ClampingOff();

	configureOpacity(data, thresholdLower, thresholdUpper);
}

void VolumePipeline::updateImageFilters(ImageData& data) const
{
	// OCT stacks are scalar intensity volumes. Some TIFF and JPEG writers
	// store visually greyscale data as RGB. Passing those three components
	// directly to vtkVolume makes VTK render them independently, while the
	// opacity transfer function below controls only the first component.
	// Convert RGB input to one luminance component before thresholding so
	// masked voxels are transparent instead of appearing black.
	if (data.image->GetNumberOfScalarComponents() == 3)
	{
		data.luminance->SetInputData(data.image);
		data.extractVOI->SetInputConnection(data.luminance->GetOutputPort());
	}
	else
	{
		data.extractVOI->SetInputData(data.image);
	}

	data.extractVOI->SetVOI(
		static_cast<int>(data.VOI[0]), static_cast<int>(data.VOI[1]) - 1,
		static_cast<int>(data.VOI[2]), static_cast<int>(data.VOI[3]) - 1,
		static_cast<int>(data.VOI[4]), static_cast<int>(data.VOI[5]) - 1);
	data.threshold->SetInputConnection(data.extractVOI->GetOutputPort());
	data.threshold->ThresholdBetween(
		static_cast<double>(data.currentMinThreshold),
		static_cast<double>(data.currentMaxThreshold));
	// Preserve every accepted source intensity so the original greyscale is
	// rendered. Only rejected voxels are replaced by the transparent mask.
	data.threshold->ReplaceInOff();
	data.threshold->ReplaceOutOn();
	data.threshold->SetOutValue(0.0);
	data.threshold->Update();
}

void VolumePipeline::updatePlane(
	ImageData& data,
	vtkRenderWindowInteractor* interactor) const
{
	data.planeWidget->SetInteractor(interactor);
	data.planeWidget->SetTextureVisibility(data.planeIsVisible ? 1 : 0);
	data.planeWidget->GetMarginProperty()->SetOpacity(
		data.planeIsVisible ? 1.0 : 0.0);
	TransformableImagePlaneWidget* transformedPlane =
		TransformableImagePlaneWidget::SafeDownCast(data.planeWidget);
	if (transformedPlane != nullptr)
		transformedPlane->SetDisplayTransform(data.transform);

	if (!data.changePlaneInput)
		return;

	data.changePlaneInput = false;
	if (!data.showPlane)
	{
		data.planeWidget->DisplayTextOff();
		data.planeWidget->Off();
		return;
	}

	data.colorMap->SetLookupTable(data.colorFun);
	// Planes deliberately use only the colour transfer function: black plane
	// pixels stay opaque, independently of the volume's zero-opacity mask.
	data.colorMap->SetOutputFormatToRGBA();
	data.colorMap->PassAlphaToOutputOff();
	data.planeWidget->SetColorMap(data.colorMap);
	data.median->SetInputConnection(data.threshold->GetOutputPort());
	data.median->SetKernelSize(
		data.checkMedian ? data.medianKernelX : 1,
		data.checkMedian ? data.medianKernelY : 1,
		data.checkMedian ? data.medianKernelZ : 1);
	data.planeWidget->SetInputConnection(data.median->GetOutputPort());

	if (data.orientChanged)
	{
		data.orientChanged = false;
		data.planeWidget->SetPlaneOrientation(data.orientIndex);
		const int* extent = data.threshold->GetOutput()->GetExtent();
		const int axis = data.orientIndex;
		data.planeWidget->SetSliceIndex(
			(extent[2 * axis] + extent[2 * axis + 1]) / 2);
	}

	data.planeWidget->GetOrigin(data.planeOrigin);
	data.planeOrientation = data.planeWidget->GetPlaneOrientation();
	data.planeWidget->GetPoint1(data.planeP1);
	data.planeWidget->GetPoint2(data.planeP2);
	data.planeWidget->SetRightButtonAction(
		vtkImagePlaneWidget::VTK_CURSOR_ACTION);
	data.planeWidget->SetRightButtonAutoModifier(
		vtkImagePlaneWidget::VTK_NO_MODIFIER);
	data.planeWidget->UpdatePlacement();
	data.planeWidget->DisplayTextOn();
	data.planeWidget->On();
}

void VolumePipeline::updateVolume(
	ImageData& data,
	vtkRenderer* renderer) const
{
	data.volumeMapper->RemoveAllClippingPlanes();
	if (data.fileChanged)
	{
		data.fileChanged = false;
		data.volume->GetProperty()->SetColor(data.colorFun);
		data.volume->GetProperty()->SetScalarOpacity(data.opacityFun);
		data.volume->GetProperty()->SetInterpolationTypeToLinear();
	}

	if (data.showPlane)
	{
		double worldOrigin[3] = {};
		double worldNormal[3] = {};
		data.transform->TransformPoint(
			data.planeWidget->GetOrigin(), worldOrigin);
		data.transform->TransformNormal(
			data.planeWidget->GetNormal(), worldNormal);
		data.clipPlane->SetOrigin(worldOrigin);
		const double direction = data.switchPlane ? -1.0 : 1.0;
		data.clipPlane->SetNormal(
			direction * worldNormal[0],
			direction * worldNormal[1],
			direction * worldNormal[2]);
		data.planeCollection->RemoveAllItems();
		data.planeCollection->AddItem(data.clipPlane);
		data.volumeMapper->SetClippingPlanes(data.planeCollection);
	}
	else
	{
		data.planeCollection->RemoveAllItems();
	}

	switch (data.blendMode)
	{
	case 1:
		data.volumeMapper->SetBlendModeToComposite();
		break;
	case 2:
		data.volumeMapper->SetBlendModeToAdditive();
		break;
	case 3:
		data.volumeMapper->SetBlendModeToMinimumIntensity();
		break;
	default:
		data.volumeMapper->SetBlendModeToMaximumIntensity();
		break;
	}

	data.volumeMapper->SetInputConnection(data.threshold->GetOutputPort());
	data.volume->SetMapper(data.volumeMapper);
	data.volume->SetScale(1.0, 1.0, 1.0);
	data.volume->SetUserTransform(data.transform);
	if (!renderer->HasViewProp(data.volume))
		renderer->AddVolume(data.volume);
	data.volume->SetVisibility(data.showObject);
}
