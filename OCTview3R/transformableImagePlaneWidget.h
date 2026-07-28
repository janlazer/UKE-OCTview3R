#pragma once

#include <vtkImagePlaneWidget.h>

class vtkLinearTransform;

class TransformableImagePlaneWidget final : public vtkImagePlaneWidget
{
public:
	static TransformableImagePlaneWidget* New();
	vtkTypeMacro(TransformableImagePlaneWidget, vtkImagePlaneWidget);

	void SetDisplayTransform(vtkLinearTransform* transform);

protected:
	TransformableImagePlaneWidget() = default;
	~TransformableImagePlaneWidget() override = default;

	void OnMouseMove() override;
	void StartCursor() override;
	void StartSliceMotion() override;
	void UpdateTransformedCursor(int x, int y);

	vtkLinearTransform* DisplayTransform = nullptr;

private:
	TransformableImagePlaneWidget(const TransformableImagePlaneWidget&) = delete;
	void operator=(const TransformableImagePlaneWidget&) = delete;
};
