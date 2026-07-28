#pragma once

class vtkRenderWindowInteractor;
class vtkRenderer;

struct ImageData;
struct Settings;

class VolumePipeline final
{
public:
	void update(
		ImageData& data,
		const Settings& settings,
		vtkRenderer* renderer,
		vtkRenderWindowInteractor* interactor) const;

private:
	void updateTransferFunctions(ImageData& data) const;
	void updateImageFilters(ImageData& data) const;
	void updatePlane(ImageData& data, vtkRenderWindowInteractor* interactor) const;
	void updateVolume(ImageData& data, vtkRenderer* renderer) const;
};
