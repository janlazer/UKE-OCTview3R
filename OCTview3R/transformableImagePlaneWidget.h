#pragma once

#include <vtkImagePlaneWidget.h>

class vtkLinearTransform;

/**
 * @brief Keep a volume's slice-plane display and interaction in the same transform.
 *
 * Plane geometry/reslicing stay in untransformed image coordinates. Display
 * actors use the parent volume's transform; mouse picks and view normals are
 * converted back to local coordinates before invoking the base widget helpers.
 * Intended for the GUI thread and VTK 8.2's protected widget API.
 */
class TransformableImagePlaneWidget final : public vtkImagePlaneWidget
{
public:
	static TransformableImagePlaneWidget* New();
	vtkTypeMacro(TransformableImagePlaneWidget, vtkImagePlaneWidget);

	/** Apply a local-to-world transform to all plane actors; nullptr means identity.
	 * The cached pointer is borrowed; the parent dataset must keep it valid while
	 * the widget is attached. A non-null transform must be invertible for picking.
	 * This changes display/interaction, not voxel spacing or the reslice input.
	 */
	void SetDisplayTransform(vtkLinearTransform* transform);

protected:
	TransformableImagePlaneWidget() = default;
	~TransformableImagePlaneWidget() override = default;

	void OnMouseMove() override;
	void StartCursor() override;
	void StartSliceMotion() override;
	/// Resolve a display-pixel pick to local image coordinates and update the crosshair.
	void UpdateTransformedCursor(int x, int y);

	vtkLinearTransform* DisplayTransform = nullptr;

private:
	TransformableImagePlaneWidget(const TransformableImagePlaneWidget&) = delete;
	void operator=(const TransformableImagePlaneWidget&) = delete;
};
