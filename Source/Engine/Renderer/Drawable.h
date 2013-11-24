// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_DRAWABLE_
#define _ENGINE_DRAWABLE_

#include "Engine\Engine\FrameTime.h"
#include "Generic\Patterns\Singleton.h"
#include "Engine\Renderer\Renderer.h"
#include "Engine\Renderer\RenderPipeline.h"

// Base class for all objects that can be drawn by the renderer.
class Drawable
{
protected:
	Camera* m_draw_camera;
	int m_render_slot_hash;

public:

	Drawable();

	// Base functions.	
	virtual void Draw(const FrameTime& time, RenderPipeline* pipeline) = 0;

	// Sets the camera that this object draws for, NULL for all.
	void	Set_Draw_Camera(Camera* camera);
	Camera* Get_Draw_Camera();

	// Sets the render slot for this object.
	void		Set_Render_Slot(const char* name);
	int			Get_Render_Slot_Hash();

};

#endif

