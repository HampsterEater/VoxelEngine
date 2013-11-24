// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FONTRENDERER_
#define _ENGINE_RENDERER_TEXT_FONTRENDERER_

#include "Generic\Types\LinkedList.h"
#include "Generic\Types\HashTable.h"
#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Point.h"
#include "Engine\IO\Stream.h"
#include "Engine\Engine\FrameTime.h"

#include "Engine\Renderer\Renderer.h"

#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontHandle.h"
#include "Engine\Renderer\Text\FontFactory.h"

#include "Engine\Renderer\Shaders\ShaderFactory.h"
#include "Engine\Renderer\Shaders\ShaderProgram.h"

#include "Generic\Types\Color.h"

struct TextAlignment
{
	enum Type
	{
		Left,
		Right,
		Center,
		Top,
		Bottom,
		Justified
	};
};

class FontRenderer 
{
private:
	FontHandle*				m_font;

	bool					m_shadow;

	bool					m_buffered;
	RenderTarget*			m_buffer_target;
	Texture*				m_buffer_texture;
	char*					m_buffer_text;
	char*					m_buffer_text_length;

public:

	// Constructors
	FontRenderer();	
	FontRenderer(FontHandle* font, bool buffered = false, bool shadow = true);	
	~FontRenderer();	

	// Rendering methods.
	void Draw_String(const FrameTime& time, const char* text, Point location, Color color = Color::White, float size = 16.0f, Point extra_spacing = Point(0, 0));
	void Draw_String(const FrameTime& time, const char* text, Rect bounds, Color color = Color::White, TextAlignment::Type horizontal_align = TextAlignment::Left, TextAlignment::Type vertical_align = TextAlignment::Top);

	Point Calculate_String_Size(const char* text, float size = 16.0f);

};

#endif

