// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontRenderer.h"
#include "Engine\Renderer\RenderPipeline.h"

#include "Engine\Renderer\Textures\Texture.h"
#include "Engine\Renderer\Textures\TextureFactory.h"

#include "Engine\Scene\Camera.h"

#include "Generic\Helper\StringHelper.h"

#include "Generic\Math\Math.h"

// TODO: Buffered rendering.

FontRenderer::FontRenderer(FontHandle* font, bool buffered)
	: m_buffered(buffered)
	, m_font(font)
	, m_buffer_target(NULL)
	, m_buffer_texture(NULL)
	, m_buffer_text(NULL)
	, m_buffer_text_length(0)
{
}

FontRenderer::~FontRenderer()
{
	SAFE_DELETE(m_buffer_target);
	SAFE_DELETE(m_buffer_texture);
	SAFE_DELETE(m_buffer_text);
}

void FontRenderer::Draw_String(const FrameTime& time, const char* text, Point location, float size, Point extra_spacing)
{
	Font*					font		= m_font->Get();
	Renderer*				renderer	= Renderer::Get();
	RenderPipeline*			pipeline	= RenderPipeline::Get();
	RenderPipeline_Shader*	shader		= pipeline->Get_Shader_From_Name("distance_field_font");
	Rect					viewport	= pipeline->Get_Active_Camera()->Get_Viewport();
	float					sdf_spread	= font->Get_SDF_Spread();
	float					scale		= size / font->Get_SDF_Source_Size();
	float					baseline	= font->Get_Baseline() * scale;
	
	// Initial rendering state for SDF.
	renderer->Set_Alpha_Test(false);
	renderer->Set_Blend(true);
	renderer->Set_Blend_Function(RendererOption::Src_Alpha_One_Minus_Src_Alpha);

	pipeline->Apply_Shader(time, shader); 
	pipeline->Update_Shader_Uniforms();
	shader->Shader_Program->Bind_Texture("g_texture", 0);
	shader->Shader_Program->Bind_Float	("g_scale", scale);
	shader->Shader_Program->Bind_Float	("g_spread", sdf_spread);

	// Draw each glyph!
	Texture* glyph_texture = NULL;
	float offset_x = location.X;
	float offset_y = location.Y + baseline - size;
	int length = strlen(text);

	for (int i = 0; i < length; i++)
	{
		unsigned char	glyph		= text[i]; // TODO: UTF-8 to UTF-32
		FontGlyph		font_glyph	= font->Get_Glyph(glyph);
	
		// Newline?
		if (glyph == '\n')
		{
			offset_y += size + extra_spacing.Y;
			offset_x = location.X;
			continue;
		}

		// Bind font glyph.
		if (font_glyph.Texture != glyph_texture)
		{
			renderer->Bind_Texture(font_glyph.Texture, 0);
			glyph_texture = font_glyph.Texture;
		}

		// Calculate position of glyph.
		Rect uv = font_glyph.UV;
		Rect rect = Rect(
				ceilf(offset_x + (font_glyph.Offset.X * scale)), 
				ceilf(offset_y + (font_glyph.Offset.Y * scale)), 
				ceilf(font_glyph.Size.X * scale), 
				ceilf(font_glyph.Size.Y * scale)
			);
		
		// Draw!
		renderer->Draw_Quad(rect, uv);

		// Advance the char offset.
		offset_x += (font_glyph.Advance.X * scale) + extra_spacing.X;
	}
	
	// Finished!
	renderer->Set_Alpha_Test(true);
}

void FontRenderer::Draw_String(const FrameTime& time, const char* text, Rect bounds, TextAlignment::Type horizontal_align, TextAlignment::Type vertical_align)
{ 
	// Has we already buffered this string?
	// TO

	// Measure string size and scale font to fit it in.
	Point string_size = Calculate_String_Size(time, text, 16.0f);

	float x_delta = string_size.X - bounds.Width;
	float y_delta = string_size.Y - bounds.Height;
	float scale	  = 1.0f;

	if (x_delta > y_delta)
	{
		scale = Min(1.0f, bounds.Width / string_size.X);
	}
	else
	{
		scale = Min(1.0f, bounds.Height / string_size.Y);
	}

	float font_size = 16.0f * scale;
	string_size = Point(string_size.X * scale, string_size.Y * scale);

	// Split into multiple lines.
	std::vector<std::string> lines;
	StringHelper::Split(text, '\n', lines);

	// Calculate Y-Offset based on alignment.
	float y_offset = 0.0f;
	float extra_spacing_y = 0.0f;
	switch (vertical_align)
	{
	case TextAlignment::Top:		
		{
			y_offset = 0;
			break;
		}
	case TextAlignment::Center:
		{
			y_offset = (bounds.Height / 2) - (string_size.Y / 2);
			break;
		}
	case TextAlignment::Bottom:		
		{
			y_offset = bounds.Height - string_size.Y;
			break;
		}
	case TextAlignment::Justified:
		{
			extra_spacing_y = (bounds.Height - string_size.Y) / lines.size();
			break;
		}
	default: DBG_ASSERT_STR(false, "Invalid vertical alignment.");
	}

	// Render each line individually with appropriate alignment.
	float pen_y = y_offset;
	for (std::vector<std::string>::iterator iter = lines.begin(); iter != lines.end(); iter++)
	{
		std::string& line = *iter;

		// Horizontal alignment.
		switch (horizontal_align)
		{
		case TextAlignment::Left:	
			{
				Draw_String(time, line.c_str(), Point(bounds.X, pen_y), font_size, Point(0, extra_spacing_y));
				break;
			}
		case TextAlignment::Center:
			{
				Point size = Calculate_String_Size(time, line.c_str(), font_size);
				Draw_String(time, line.c_str(), Point((bounds.X + (bounds.Width / 2)) - (size.X / 2), pen_y), font_size, Point(0, extra_spacing_y));
				break;
			}
		case TextAlignment::Right:	
			{
				Point size = Calculate_String_Size(time, line.c_str(), font_size);
				Draw_String(time, line.c_str(), Point(bounds.X + bounds.Width - size.X, pen_y), font_size, Point(0, extra_spacing_y));
				break;
			}		
		case TextAlignment::Justified:
			{
				Point size = Calculate_String_Size(time, line.c_str(), font_size);
				float extra_spacing = (bounds.Width - size.X) / line.length(); // TODO: Utf8 decoding for length.
				Draw_String(time, line.c_str(), Point(bounds.X, pen_y), font_size, Point(extra_spacing, 0));
				break;
			}
		default: DBG_ASSERT_STR(false, "Invalid horizontal alignment.");
		}
		
		pen_y += font_size + extra_spacing_y;
	}
}

Point FontRenderer::Calculate_String_Size(const FrameTime& time, const char* text, float size)
{
	Font*	font		= m_font->Get();
	float	sdf_spread	= font->Get_SDF_Spread();
	float	scale		= size / font->Get_SDF_Source_Size();
	float	baseline	= font->Get_Baseline() * scale;

	// Draw each glyph!
	Texture* glyph_texture = NULL;
	float max_x			= 0;
	float max_y			= 0;
	float offset_x		= 0;
	float offset_y		= baseline - size;
	float line_height	= size;
	int   length		= strlen(text);

	for (int i = 0; i < length; i++)
	{
		unsigned char	glyph		= text[i]; // TODO: UTF-8 to UTF-32
		FontGlyph		font_glyph	= font->Get_Glyph(glyph);
	
		// Newline?
		if (glyph == '\n')
		{
			offset_y += size;
			offset_x = 0;
			continue;
		}

		// Calculate position of glyph.
		Rect rect = Rect(
				ceilf(offset_x + (font_glyph.Offset.X * scale)), 
				ceilf(offset_y + (font_glyph.Offset.Y * scale)), 
				ceilf(font_glyph.Size.X * scale), 
				ceilf(font_glyph.Size.Y * scale)
			);		
		
		// Measure size.
		if (rect.Y + rect.Height > max_y)
			max_y = rect.Y + rect.Height;
		if (rect.X + rect.Width > max_x)
			max_x = rect.X + rect.Width;

		// Advance the char offset.
		offset_x += (font_glyph.Advance.X * scale);
	}

	return Point(max_x, max_y);
}
