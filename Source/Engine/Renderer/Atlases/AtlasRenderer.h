// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_ATLASES_ATLASRENDERER_
#define _ENGINE_RENDERER_ATLASES_ATLASRENDERER_

#include "Generic\Types\LinkedList.h"
#include "Generic\Types\HashTable.h"
#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Point.h"
#include "Generic\Types\Color.h"
#include "Engine\IO\Stream.h"
#include "Engine\Engine\FrameTime.h"

#include "Engine\Renderer\Renderer.h"

#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\AtlasFactory.h"

#include "Engine\Renderer\Shaders\ShaderFactory.h"
#include "Engine\Renderer\Shaders\ShaderProgram.h"

class AtlasRenderer 
{
private:
	AtlasHandle* m_atlas;

public:

	// Constructors
	AtlasRenderer();
	AtlasRenderer(AtlasHandle* atlas);	
	~AtlasRenderer();	

	// Rendering methods.
	void Draw_Frame	(const FrameTime& time, AtlasFrame* frame, Point point, Color color = Color::White);
	void Draw_Frame	(const FrameTime& time, AtlasFrame* frame, Rect  point, Color color = Color::White);
	void Draw_Frame	(const FrameTime& time, const char* frame, Point point, Color color = Color::White);
	void Draw_Frame	(const FrameTime& time, const char* frame, Rect  point, Color color = Color::White);

};

#endif

