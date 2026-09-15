#pragma once

class vtkRenderer;

struct ImageData;
struct Settings;

/**
 * @brief GUI-thread updater for document-owned geometry, clipping and material.
 *
 * Crops in the mesh's native coordinates before applying its display transform.
 * No automatic unit conversion or registration to independently loaded OCT data
 * is performed. Surface meshes use generated normals; non-surface inputs bypass
 * that stage. The selected solid colour overrides input scalar colouring.
 */
class PolyPipeline final
{
public:
	/** Apply dirty fields and ensure the actor belongs to the renderer.
	 * @pre data is loaded PolyData with DocumentModel-created resources; renderer is valid.
	 * @post All dirty flags are cleared; neither Render() nor camera reset is called.
	 * settings is currently unused and retained for the common pipeline interface.
	 */
	void update(
		ImageData& data,
		const Settings& settings,
		vtkRenderer* renderer) const;
};
