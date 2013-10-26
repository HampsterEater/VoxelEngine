// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Renderer.h"
#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontFactory.h"
#include "Engine\Renderer\Text\FreeType\FreeType_Font.h"
#include "Engine\Renderer\Text\FreeType\FreeType_FontFactory.h"

#include "Engine\Renderer\Textures\TextureFactory.h"

#include "Engine\Config\ConfigFile.h"

#include "Generic\Math\Math.h"

#include "Engine\Localise\Locale.h"

FreeType_Font::FreeType_Font(FT_Library library, FT_Face face, char* buffer, ConfigFile* config)
	: m_library(library)
	, m_face(face)
	, m_textures_locked(false)
	, m_current_dirty_texture_index(-1)
	, m_buffer(buffer)
{
	// Load configuration.
	m_sdf_spread	= config->Get<int>("generation/sdf-spread");
	m_sdf_downscale = config->Get<int>("generation/sdf-downscale");
	m_sdf_threshold = config->Get<int>("generation/sdf-threshold");
	m_texture_size  = config->Get<int>("generation/texture-size");
	m_max_textures	= config->Get<int>("generation/max-textures");
	m_glyph_size    = config->Get<int>("generation/glyph-size");
	m_glyph_spacing = config->Get<int>("generation/glyph-spacing");
	
	// Create texture buffer.
	m_textures = new FreeType_FontTexture*[m_max_textures];
	memset(m_textures, 0, m_max_textures * sizeof(Texture*));	

	// Get default language.
	const char* glyphs = S("glyphs");
	int length = strlen(glyphs);

	// Set face to use our character size.
	int result = FT_Set_Pixel_Sizes(face, m_glyph_size * m_sdf_downscale, m_glyph_size * m_sdf_downscale);
	DBG_ASSERT_STR(result == 0, "Failed to set character size of freetype font.");

	// Create texture.
	Lock_Textures();
	Add_Glyphs(glyphs);
	Unlock_Textures();
}

FreeType_Font::FreeType_Font(FT_Library library, FT_Face face, char* buffer)
	: m_library(library)
	, m_face(face)
	, m_textures_locked(false)
	, m_current_dirty_texture_index(-1)
	, m_buffer(buffer)
{
}

FreeType_Font::~FreeType_Font()
{
	for (int i = 0; i < m_max_textures; i++)
	{
		FreeType_FontTexture* texture = m_textures[i];
		if (texture != NULL)
		{
			SAFE_DELETE(texture);
		}
	}

	SAFE_DELETE(m_textures);

	FT_Done_Face(m_face);

	m_library = NULL;

	SAFE_DELETE(m_buffer);
}

bool FreeType_Font::Load_Compiled_Config(ConfigFile* config)
{	
	// Load configuration.
	m_sdf_spread	= config->Get<int>("generation/sdf-spread");
	m_sdf_downscale = config->Get<int>("generation/sdf-downscale");
	m_sdf_threshold = config->Get<int>("generation/sdf-threshold");
	m_texture_size  = config->Get<int>("generation/texture-size");
	m_max_textures	= config->Get<int>("generation/max-textures");
	m_glyph_size    = config->Get<int>("generation/glyph-size");
	m_glyph_spacing = config->Get<int>("generation/glyph-spacing");
	
	// Create texture buffer.
	m_textures = new FreeType_FontTexture*[m_max_textures];
	memset(m_textures, 0, m_max_textures * sizeof(Texture*));	

	// Set face to use our character size.
	int result = FT_Set_Pixel_Sizes(m_face, m_glyph_size * m_sdf_downscale, m_glyph_size * m_sdf_downscale);
	DBG_ASSERT_STR(result == 0, "Failed to set character size of freetype font.");

	// Load in all textures.
	std::vector<const char*> textures = config->Get<std::vector<const char*>>("textures/texture");
	DBG_ASSERT_STR(textures.size() < m_max_textures, "Compiled font contains more textures that max-textures.");

	for (std::vector<const char*>::iterator iter = textures.begin(); iter != textures.end(); iter++)
	{
		m_current_dirty_texture_index++;

		m_textures[m_current_dirty_texture_index]			= new FreeType_FontTexture();
		m_textures[m_current_dirty_texture_index]->Is_Dirty = false;
		m_textures[m_current_dirty_texture_index]->Texture	= TextureFactory::Load_Without_Handle(*iter, TextureFlags::None);

		if (m_textures[m_current_dirty_texture_index]->Texture == NULL)
		{
			DBG_LOG("Failed to load compiled texture '%s'.", *iter);
			return false;
		}

		m_textures[m_current_dirty_texture_index]->Buffer	= (unsigned char*)m_textures[m_current_dirty_texture_index]->Texture->Get_Data();
		m_textures[m_current_dirty_texture_index]->Packer   = RectanglePacker(0, 0);		
	}

	// We don't want to try appending glyphs to last loaded texture, so move to the "next" one.
	m_current_dirty_texture_index++;

	// Load all glyphs.
	std::vector<ConfigFileNode> glyphs = config->Get<std::vector<ConfigFileNode>>("glyphs/glyph");
	for (std::vector<ConfigFileNode>::iterator iter = glyphs.begin(); iter != glyphs.end(); iter++)
	{
		ConfigFileNode node = *iter;

		int texture_index = config->Get<int>("texture", node, true);
		DBG_ASSERT_STR(texture_index < m_max_textures, "Glyph attempted to reference out of bounds texture index '%i'.", texture_index);

		// Add glyph to list.	
		FreeType_FontGlyph* new_glyph	= new FreeType_FontGlyph();	
		new_glyph->FreeType_GlyphIndex	= config->Get<unsigned int>("ftindex", node, true);
		new_glyph->Glyph.Texture		= m_textures[texture_index]->Texture;		
		new_glyph->Glyph.Glyph			= config->Get<unsigned int>("glyph", node, true);
		new_glyph->Glyph.UV				= config->Get<Rect>("uv", node, true);
		new_glyph->TextureIndex			= texture_index;
		new_glyph->Glyph.Size			= config->Get<Point>("size", node, true);
		new_glyph->Glyph.Advance		= config->Get<Point>("advance", node, true);
		new_glyph->Glyph.Offset			= config->Get<Point>("offset", node, true);
		new_glyph->Glyph.Baseline		= config->Get<float>("baseline", node, true);

		m_glyphs.Set(new_glyph->Glyph.Glyph, new_glyph);
	}

	return false;
}

FontGlyph FreeType_Font::Get_Glyph(unsigned int glyph) 
{
	FreeType_FontGlyph* output;
	if (m_glyphs.Get(glyph, output))
	{
		return output->Glyph;
	}
	else
	{
		Lock_Textures();
		Add_Glyph(glyph);
		Unlock_Textures();

		bool result = m_glyphs.Get(glyph, output);
		DBG_ASSERT(result == true);

		return output->Glyph;
	}
}

void FreeType_Font::Lock_Textures()
{
	m_textures_locked = true;
}

void FreeType_Font::Unlock_Textures()
{
	m_textures_locked = false;

	for (int i = 0; i < m_max_textures; i++)
	{
		FreeType_FontTexture* texture = m_textures[i];
		if (texture != NULL && texture->Is_Dirty == true)
		{
			texture->Texture->Set_Data((char*)texture->Buffer);
			texture->Is_Dirty = false;
		}
	}
}

FreeType_FontTexture** FreeType_Font::Get_Textures(int& texture_count)
{
	texture_count = m_current_dirty_texture_index + 1;
	return m_textures;
}

float FreeType_Font::Get_SDF_Spread()
{
	return m_sdf_spread;
}

float FreeType_Font::Get_SDF_Source_Size()
{
	return m_glyph_size;
}

float FreeType_Font::Get_Baseline()
{
	return float(m_face->ascender) / m_sdf_downscale;
}

void FreeType_Font::Add_Glyphs(const char* glyphs)
{
	DBG_ASSERT(m_textures_locked == true);

	Renderer* renderer = Renderer::Get();

	int length = strlen(glyphs);
	for (int i = 0; i < length; i++)
	{
		unsigned int glyph = (unsigned int)((unsigned char)glyphs[i]); // TODO utf8-to-utf32
		Add_Glyph(glyph);
	}
}

Point FreeType_Font::Get_Kerning(unsigned int prev, unsigned int next)
{
	FreeType_FontGlyph* prev_glyph = m_glyphs.Get(prev);
	FreeType_FontGlyph* next_glyph = m_glyphs.Get(next);

	FT_Vector kerning;

	int result = FT_Get_Kerning(m_face, prev_glyph->FreeType_GlyphIndex, next_glyph->FreeType_GlyphIndex, FT_KERNING_DEFAULT, &kerning);
	DBG_ASSERT(result == 0);

	return Point(kerning.x, kerning.y);
}

HashTable<FreeType_FontGlyph*, unsigned int>& FreeType_Font::Get_Glyphs()
{
	return m_glyphs;
}

void FreeType_Font::Add_Glyph(unsigned int glyph)
{
	Renderer* renderer = Renderer::Get();

	// Check it dosen't already exist.
	if (m_glyphs.Contains(glyph))
	{
		return;
	}
	
	// Generate glyph with freetype.
	int ft_glyph_index = FT_Get_Char_Index(m_face, glyph);
	if (ft_glyph_index <= 0)
	{
		DBG_LOG("Undefined character 0x%08x in font.", glyph);

		// Use a question mark.
		ft_glyph_index = FT_Get_Char_Index(m_face, '?');
	
		DBG_ASSERT_STR(ft_glyph_index > 0, "No question mark in font!", glyph);
	}

	// Load in the glyph.
	int result = FT_Load_Glyph(m_face, ft_glyph_index, FT_LOAD_RENDER);
	DBG_ASSERT_STR(result == 0, "Unable to load character 0x%08x from freetype.", ft_glyph_index);

	// Work out position on texture.
	bool  new_texture = false;
	Point glyph_size_correct_scale = Point((m_face->glyph->bitmap.width / m_sdf_downscale), (m_face->glyph->bitmap.rows / m_sdf_downscale));
	Point glyph_size = Point(glyph_size_correct_scale.X + (m_glyph_spacing * 2), glyph_size_correct_scale.Y + (m_glyph_spacing * 2));
	Rect  glyph_rect = Rect(0, 0, 0, 0);

	if (m_current_dirty_texture_index < 0)
	{
		new_texture = true;
	}
	else
	{
		new_texture = !m_textures[m_current_dirty_texture_index]->Packer.Pack(glyph_size, glyph_rect);
	}

	if (new_texture == true)
	{	
		DBG_ASSERT_STR(++m_current_dirty_texture_index < m_max_textures, "Ran out of font texture space.");

		m_textures[m_current_dirty_texture_index] = new FreeType_FontTexture();
		m_textures[m_current_dirty_texture_index]->Packer = RectanglePacker(m_texture_size, m_texture_size);
		m_textures[m_current_dirty_texture_index]->Buffer = new unsigned char[m_texture_size * m_texture_size];
		memset(m_textures[m_current_dirty_texture_index]->Buffer, 0, m_texture_size * m_texture_size);
		m_textures[m_current_dirty_texture_index]->Texture = renderer->Create_Texture(m_texture_size, m_texture_size, m_texture_size, TextureFormat::Luminosity, TextureFlags::LinearFilter);
	
		bool packed = m_textures[m_current_dirty_texture_index]->Packer.Pack(glyph_size, glyph_rect);
		DBG_ASSERT_STR(packed, "Could not pack glyph into brand new texture! To large?");
	}

	// Work out glyph texture position etc.
	int pixel_x				= glyph_rect.X;
	int pixel_y				= glyph_rect.Y;
	int pixel_w				= glyph_rect.Width;
	int pixel_h				= glyph_rect.Height;
		
	float uv_w				= (glyph_rect.Width) / float(m_texture_size);		
	float uv_h				= (glyph_rect.Height) / float(m_texture_size);
	float uv_x				= (glyph_rect.X) / float(m_texture_size);
	float uv_y				= (glyph_rect.Y) / float(m_texture_size);

	// Only render if we have a bitmap for this glyph.
	if (m_face->glyph->bitmap.buffer != NULL)
	{
		FreeType_FontTexture* texture = m_textures[m_current_dirty_texture_index];

		// Calculate rectangle to calculate SDF for.
		int sdf_rect_x  = pixel_x;
		int sdf_rect_y  = pixel_y;
		int sdf_rect_ex = pixel_x + pixel_w;
		int sdf_rect_ey = pixel_y + pixel_h;

		int glyph_origin_x = pixel_x + m_glyph_spacing;
		int glyph_origin_y = pixel_y + m_glyph_spacing;

		// Calculate SDF for all pixels in area.
		for (int y = sdf_rect_y; y < sdf_rect_ey; y++)
		{
			for (int x = sdf_rect_x; x < sdf_rect_ex; x++)
			{
				int input_x = ((x - glyph_origin_x) * m_sdf_downscale) + (m_sdf_downscale / 2);
				int input_y = ((y - glyph_origin_y) * m_sdf_downscale) + (m_sdf_downscale / 2);

				// Note: Inversed Y-Axis
				unsigned char* out_pixel = texture->Buffer + (m_texture_size * (m_texture_size - (y + 1))) + (x);
				
				float			distance = Find_SDF_Distance(input_x, input_y);
				unsigned char	alpha	 = SDF_Distance_To_Alpha(distance);

				*out_pixel = alpha;	
			}
		}
	}

	// Add glyph to list.	
	FreeType_FontGlyph* new_glyph = new FreeType_FontGlyph();	
	new_glyph->FreeType_GlyphIndex = ft_glyph_index;
	new_glyph->Glyph.Texture	= m_textures[m_current_dirty_texture_index]->Texture;		
	new_glyph->Glyph.Glyph		= glyph;
	new_glyph->Glyph.UV			= Rect(uv_x, uv_y, uv_w, uv_h);
	new_glyph->TextureIndex		= m_current_dirty_texture_index;
	new_glyph->Glyph.Size		= Point(glyph_size_correct_scale.X, glyph_size_correct_scale.Y);
	new_glyph->Glyph.Advance	= Point((m_face->glyph->advance.x >> 6) / m_sdf_downscale, (m_face->glyph->advance.y >> 6) / m_sdf_downscale);
	new_glyph->Glyph.Offset		= Point(m_face->glyph->bitmap_left / m_sdf_downscale, -(m_face->glyph->bitmap_top / m_sdf_downscale));

	m_textures[m_current_dirty_texture_index]->Is_Dirty = true;

	m_glyphs.Set(glyph, new_glyph);
}

unsigned char FreeType_Font::SDF_Distance_To_Alpha(float distance)
{
	float div = (distance / m_sdf_spread);
	float x = (0.5f * div);
	float alpha = 0.5f + x;
	alpha = Min(1.0f, Max(0.0, alpha)); 

	return (unsigned char)(alpha * 0xFF);
}

float FreeType_Font::Find_SDF_Distance(int center_x, int center_y)
{
	int width = m_face->glyph->bitmap.width;
	int rows  = m_face->glyph->bitmap.rows; 

	bool base_pixel = false;
	if (center_x >= 0 && center_y >= 0 && center_x < width && center_y < rows)
	{
		base_pixel = *(m_face->glyph->bitmap.buffer + (center_y * width) + center_x) > m_sdf_threshold;
	}

	int delta	= (int)ceilf(m_sdf_spread);
	int start_x = Max(0, center_x - delta);
	int end_x	= Min(width - 1, center_x + delta);
	int start_y = Max(0, center_y - delta);
	int end_y	= Min(rows - 1, center_y + delta);

	int closest_distance = delta * delta;

	// Find closest pixel of inverse state from this one.
	for (int y = start_y; y <= end_y;  y++)
	{
		for (int x = start_x; x <= end_x; x++)
		{
			bool this_pixel = false;
			if (x >= 0 && y >= 0 && x < width && y < rows)
			{
				this_pixel = *(m_face->glyph->bitmap.buffer + (y * width) + x) > m_sdf_threshold;
			}

			if (base_pixel != this_pixel)
			{
				int delta_x = center_x - x;
				int delta_y = center_y - y;
				int square_dist = delta_x * delta_x + delta_y * delta_y;

				if (square_dist < closest_distance)
				{
					closest_distance = square_dist;
				}
			}
		}
	}

	float closest_dist = sqrtf(closest_distance);
	return (base_pixel ? 1 : -1) * Min(closest_dist, m_sdf_spread);
}
