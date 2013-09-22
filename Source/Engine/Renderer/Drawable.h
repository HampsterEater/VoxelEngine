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
public:

	// Base functions.	
	virtual void Draw(const FrameTime& time, RenderPipeline* pipeline) = 0;

};

#endif

