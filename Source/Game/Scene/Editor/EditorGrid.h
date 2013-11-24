// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_EDITOR_EDITOR_GRID_
#define _ENGINE_SCENE_EDITOR_EDITOR_GRID_

#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Point.h"

#include "Engine\Scene\Actor.h"
#include "Engine\Scene\Camera.h"
#include "Engine\Scene\Tickable.h"

#include "Engine\Renderer\Drawable.h"

class EditorGrid : public Actor, public Tickable, public Drawable
{
private:
	float m_grid_size;
	float m_grid_expanse;
	bool m_mesh_dirty;
	int m_mesh_id;

public:
	EditorGrid();

	// Base functions.
	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time, RenderPipeline* pipeline);

};

#endif

