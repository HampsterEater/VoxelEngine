// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_EDITOR_EDITOR_SELECTION_PRIMITIVE_
#define _ENGINE_SCENE_EDITOR_EDITOR_SELECTION_PRIMITIVE_

#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Point.h"

#include "Engine\Scene\Actor.h"
#include "Engine\Scene\Camera.h"
#include "Engine\Scene\Tickable.h"

#include "Engine\Renderer\Drawable.h"

class EditorSelectionPrimitive : public Actor, public Tickable, public Drawable
{
private:
	Vector3 m_size;

public:
	EditorSelectionPrimitive();

	// Base functions.
	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time, RenderPipeline* pipeline);

	// Size settings.
	void	Set_Size(Vector3 size);
	Vector3 Get_Size();

};

#endif

