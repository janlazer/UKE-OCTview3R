#pragma once

class vtkRenderer;

struct ImageData;
struct Settings;

class PolyPipeline final
{
public:
	void update(
		ImageData& data,
		const Settings& settings,
		vtkRenderer* renderer) const;
};
