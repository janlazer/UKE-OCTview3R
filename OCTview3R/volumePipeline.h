#pragma once

class vtkRenderWindowInteractor;
class vtkRenderer;

struct ImageData;
struct Settings;

/**
 * @brief Stateless updater of one document's scalar/RGB volume and slice plane.
 *
 * All filters, props and settings belong to ImageData. Source voxels are never
 * edited: crop/threshold/RGB branches produce derived outputs. Use on the GUI
 * thread after loading has completed; VTK updates may allocate large buffers.
 */
class VolumePipeline final
{
public:
	/** Apply dirty state in transform -> transfer function -> filters -> plane -> volume order.
	 * @pre data is a loaded volume with resources created by DocumentModel::create().
	 * @pre renderer/interactor are valid and belong to the shared GUI scene.
	 * @post Dirty flags are consumed; does not call Render() or reset the camera.
	 * Callers must mark every affected flag; update() does not infer dependencies
	 * from edited fields. settings is reserved for the common pipeline interface.
	 */
	void update(
		ImageData& data,
		const Settings& settings,
		vtkRenderer* renderer,
		vtkRenderWindowInteractor* interactor) const;

private:
	/// Separate scalar palette/window mapping from opacity; RGB uses its alpha component.
	void updateTransferFunctions(ImageData& data) const;
	/// Convert half-open voxel bounds to VTK extents and wire scalar or RGBA outputs.
	void updateImageFilters(ImageData& data) const;
	/// Attach transform/interactor; reconnect plane input only when changePlaneInput is set.
	void updatePlane(ImageData& data, vtkRenderWindowInteractor* interactor) const;
	/// Select mapper input/material, world-space clipping, blend mode and visibility.
	void updateVolume(ImageData& data, vtkRenderer* renderer) const;
};
