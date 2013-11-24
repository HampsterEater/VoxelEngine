// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_QUADRENDERER_
#define _ENGINE_QUADRENDERER_

#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Color.h"

#include "Engine\Engine\FrameTime.h"

class Texture;
class TextureHandle;

// Just wraps up functionality for rendering several basic primitives.
class PrimitiveRenderer
{
private:
	void Apply_Primitive_Shader	(const FrameTime& time, Color color);

public:
	PrimitiveRenderer();

	void Draw_Solid_Quad		(const FrameTime& time, Rect xy, Color color);
	void Draw_Line				(const FrameTime& time, Vector3 start_zyz, Vector3 end_xyz, float size, Color color);
	void Draw_Cube				(const FrameTime& time, Vector3 size, Color color);
	void Draw_Wireframe_Cube	(const FrameTime& time, Vector3 size, float line_size, Color color);
	void Draw_Wireframe_Sphere	(const FrameTime& time, float r, float line_size, Color color);
	void Draw_Arrow				(const FrameTime& time, Vector3 direction, float line_length = 0.2f, float pip_length = 0.04f, float line_size = 1.0f, Color color = Color::White);

};

#endif

