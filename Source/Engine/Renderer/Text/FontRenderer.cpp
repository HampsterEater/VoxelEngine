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

FontRenderer::FontRenderer()
	: m_buffered(NULL)
	, m_font(NULL)
	, m_buffer_target(NULL)
	, m_buffer_texture(NULL)
	, m_buffer_text(NULL)
	, m_buffer_text_length(0)
	, m_shadow(true)
{
}

FontRenderer::FontRenderer(FontHandle* font, bool buffered, bool shadow)
	: m_buffered(buffered)
	, m_font(font)
	, m_buffer_target(NULL)
	, m_buffer_texture(NULL)
	, m_buffer_text(NULL)
	, m_buffer_text_length(0)
	, m_shadow(shadow)
{
}

FontRenderer::~FontRenderer()
{
	SAFE_DELETE(m_buffer_target);
	SAFE_DELETE(m_buffer_texture);
	SAFE_DELETE(m_buffer_text);
}

void FontRenderer::Draw_String(const FrameTime& time, const char* text, Point location, Color color, float size, Point extra_spacing)
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
	shader->Shader_Program->Bind_Texture("g_texture",		0);
	shader->Shader_Program->Bind_Float	("g_scale",			scale);
	shader->Shader_Program->Bind_Float	("g_spread",		sdf_spread);
	shader->Shader_Program->Bind_Vector	("g_color",			color.To_Vector4());	
	shader->Shader_Program->Bind_Bool	("g_draw_shadow",	m_shadow);

	// Draw each glyph!
	Texture* glyph_texture = NULL;
	float pen_x = (location.X);
	float pen_y = (location.Y + baseline);// - (baseline * scale);// - size;
	int length = strlen(text); 

	float glyph_padding_px = 2;

	for (int i = 0; i < length; i++)
	{
		unsigned char	glyph		= text[i]; // TODO: UTF-8 to UTF-32
		FontGlyph		font_glyph	= font->Get_Glyph(glyph);
	
		// Newline?
		if (glyph == '\n')
		{
			pen_y += size + 1 + extra_spacing.Y;
			pen_x = location.X;
			continue;
		}

		// Bind font glyph.
		if (font_glyph.Texture != glyph_texture)
		{
			renderer->Bind_Texture(font_glyph.Texture, 0);
			glyph_texture = font_glyph.Texture;
		}
			
		// Calculate padding.
		float glyph_padding_uv = (glyph_padding_px / glyph_texture->Get_Width()) ;

		// Calculate position of glyph.
		// TODO: Pad glyph
		Rect uv = font_glyph.UV;
		uv.X		-= glyph_padding_uv;
		uv.Y		+= glyph_padding_uv;
		uv.Width	+= glyph_padding_uv * 4;
		uv.Height	+= glyph_padding_uv * 4;

		Rect rect = Rect(
				((pen_x) + (font_glyph.Offset.X * scale)) - glyph_padding_px, 
				((pen_y) + (font_glyph.Offset.Y * scale)) - glyph_padding_px, 
				Max((font_glyph.Size.X * scale), 2) + (glyph_padding_px*2), 
				(font_glyph.Size.Y * scale) + (glyph_padding_px*2)
			);
		
		// Draw!
		renderer->Draw_Quad(rect, uv);

		// Advance the char offset.
		pen_x += (font_glyph.Advance.X * scale) + extra_spacing.X;
	}
	
	// Finished!
	renderer->Set_Alpha_Test(true);
}

void FontRenderer::Draw_String(const FrameTime& time, const char* text, Rect bounds, Color color, TextAlignment::Type horizontal_align, TextAlignment::Type vertical_align)
{ 
	// Has we already buffered this string?
	// TO

	// Measure string size and scale font to fit it in.
	Point string_size = Calculate_String_Size(text, 16.0f);

	float x_delta = abs(string_size.X - bounds.Width);
	float y_delta = abs(string_size.Y - bounds.Height);
	float scale	  = 1.0f;

//	if (x_delta > y_delta)
//	{
//		scale = Min(1.0f, bounds.Width / string_size.X);
//	}
//	else
//	{
		scale = Min(1.0f, bounds.Height / string_size.Y);
//	}

	//scale = 16.0f;

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
	float pen_y = bounds.Y + y_offset;
	for (std::vector<std::string>::iterator iter = lines.begin(); iter != lines.end(); iter++)
	{
		std::string& line = *iter;

		// Horizontal alignment.
		switch (horizontal_align)
		{
		case TextAlignment::Left:	
			{
				Draw_String(time, line.c_str(), Point(bounds.X, pen_y), color, font_size, Point(0, extra_spacing_y));
				break;
			}
		case TextAlignment::Center:
			{
				Point size = Calculate_String_Size(line.c_str(), font_size);
				Draw_String(time, line.c_str(), Point((bounds.X + (bounds.Width / 2)) - (size.X / 2), pen_y), color, font_size, Point(0, extra_spacing_y));
				break;
			}
		case TextAlignment::Right:	
			{
				Point size = Calculate_String_Size(line.c_str(), font_size);
				Draw_String(time, line.c_str(), Point(bounds.X + bounds.Width - size.X, pen_y), color, font_size, Point(0, extra_spacing_y));
				break;
			}		
		case TextAlignment::Justified:
			{
				Point size = Calculate_String_Size(line.c_str(), font_size);
				float extra_spacing = (bounds.Width - size.X) / line.length(); // TODO: Utf8 decoding for length.
				Draw_String(time, line.c_str(), Point(bounds.X, pen_y), color, font_size, Point(extra_spacing, 0));
				break;
			}
		default: DBG_ASSERT_STR(false, "Invalid horizontal alignment.");
		}
		
		pen_y += font_size + 1 + extra_spacing_y;
	}
}

Point FontRenderer::Calculate_String_Size(const char* text, float size)
{
	Font*	font		= m_font->Get();
	float	sdf_spread	= font->Get_SDF_Spread();
	float	scale		= size / font->Get_SDF_Source_Size();
	float	baseline	= font->Get_Baseline() * scale;

	// Draw each glyph!
	Texture* glyph_texture = NULL;
	float max_x			= 0;
	float max_y			= 0;
	float line_height	= font->Get_SDF_Source_Size();
	float pen_x			= 0;
	float pen_y			= line_height * scale;
	int   length		= strlen(text);

	for (int i = 0; i < length; i++)
	{
		unsigned char	glyph		= text[i]; // TODO: UTF-8 to UTF-32
		FontGlyph		font_glyph	= font->Get_Glyph(glyph);
	
		// Newline?
		if (glyph == '\n')
		{
			pen_y += line_height;
			pen_x = 0;
			continue;
		}

		// Calculate position of glyph.
		Rect rect = Rect(
				((pen_x) + (font_glyph.Offset.X * scale)), 
				((pen_y) + (font_glyph.Offset.Y * scale)), 
				Max((font_glyph.Size.X * scale), 2), 
				(font_glyph.Size.Y * scale)
			);
		
		// Measure size.
		if (pen_y > max_y)
			max_y = pen_y;
		if (rect.X + rect.Width > max_x)
			max_x = rect.X + rect.Width;

		// Advance the char offset.
		pen_x += (font_glyph.Advance.X * scale);
	}

	return Point(max_x, max_y);
}
