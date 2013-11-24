// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Renderer.h"
#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontFactory.h"
#include "Engine\Renderer\Text\FreeType\FreeType_Font.h"
#include "Engine\Renderer\Text\FreeType\FreeType_FontFactory.h"

#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Textures\Pixmap.h"
#include "Engine\Renderer\Textures\PixmapFactory.h"

#include "Engine\Platform\Platform.h"

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
	const char* glyphs = S("#glyphs");
	int length = strlen(glyphs);

	// Set face to use our character size.
#ifdef SDF_DOWNSCALE
	int result = FT_Set_Pixel_Sizes(face, m_glyph_size * m_sdf_downscale, m_glyph_size * m_sdf_downscale);
#else
	int result = FT_Set_Pixel_Sizes(face, m_glyph_size, m_glyph_size);
#endif
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
#ifdef SDF_DOWNSCALE
	int result = FT_Set_Pixel_Sizes(m_face, m_glyph_size * m_sdf_downscale, m_glyph_size * m_sdf_downscale);
#else
	int result = FT_Set_Pixel_Sizes(m_face, m_glyph_size, m_glyph_size);
#endif
	DBG_ASSERT_STR(result == 0, "Failed to set character size of freetype font.");

	// Load in all textures.
	std::vector<const char*> textures = config->Get<std::vector<const char*>>("textures/texture");
	DBG_ASSERT_STR(textures.size() <= m_max_textures, "Compiled font contains more textures that max-textures.");

	for (std::vector<const char*>::iterator iter = textures.begin(); iter != textures.end(); iter++)
	{
		m_current_dirty_texture_index++;

		m_textures[m_current_dirty_texture_index]			= new FreeType_FontTexture();
		m_textures[m_current_dirty_texture_index]->Is_Dirty = false;
		m_textures[m_current_dirty_texture_index]->Texture	= TextureFactory::Load_Without_Handle(*iter, TextureFlags::LinearFilter);

		if (m_textures[m_current_dirty_texture_index]->Texture == NULL)
		{
			DBG_LOG("Failed to load compiled texture '%s'.", *iter);
			return false;
		}

		m_textures[m_current_dirty_texture_index]->Pixmap	= m_textures[m_current_dirty_texture_index]->Texture->Get_Pixmap();
		m_textures[m_current_dirty_texture_index]->Packer   = RectanglePacker(0, 0, m_glyph_spacing);		
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
		
		if (abs(new_glyph->Glyph.Offset.Y) > m_baseline)
		{
			m_baseline = abs(new_glyph->Glyph.Offset.Y);
		}

		m_glyphs.Set(new_glyph->Glyph.Glyph, new_glyph);
	}

	return true;
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
			texture->Texture->Set_Pixmap(texture->Pixmap);
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
	return m_baseline;
}

void FreeType_Font::Add_Glyphs(const char* glyphs)
{
	DBG_ASSERT(m_textures_locked == true);

	Renderer* renderer = Renderer::Get();

	float time_accumulator = 0.0f;
	float time_counter = 0;

	int length = strlen(glyphs);
	for (int i = 0; i < length; i++)
	{
		unsigned int glyph = (unsigned int)((unsigned char)glyphs[i]); // TODO utf8-to-utf32

		float time = Platform::Get()->Get_Ticks();

		Add_Glyph(glyph);

		time_accumulator += (Platform::Get()->Get_Ticks() - time);
		time_counter++;

		float time_per_glyph = time_accumulator / time_counter;
		float minutes_left = ((time_per_glyph * (length - i)) / 1000) / 60;

		DBG_LOG("Added glyph %i of %i (Estimated %.2fm remaining).", i, length, minutes_left);
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
#ifdef SDF_DOWNSCALE
	Point glyph_size = Point(ceil(m_face->glyph->bitmap.width / m_sdf_downscale), ceil(m_face->glyph->bitmap.rows / m_sdf_downscale));
#else
	Point glyph_size = Point(m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows);
#endif
	Rect  glyph_rect = Rect(0, 0, 0, 0);

	if (m_current_dirty_texture_index < 0)
	{
		new_texture = true;
	}
	else
	{
		new_texture = m_textures[m_current_dirty_texture_index] == NULL || !m_textures[m_current_dirty_texture_index]->Packer.Pack(glyph_size, glyph_rect);
	}

	if (new_texture == true)
	{	
		DBG_ASSERT_STR(++m_current_dirty_texture_index < m_max_textures, "Ran out of font texture space.");

		m_textures[m_current_dirty_texture_index] = new FreeType_FontTexture();
		m_textures[m_current_dirty_texture_index]->Packer = RectanglePacker(m_texture_size, m_texture_size, m_glyph_spacing);
		m_textures[m_current_dirty_texture_index]->Pixmap = new Pixmap(m_texture_size, m_texture_size, PixmapFormat::R8G8B8A8);
		m_textures[m_current_dirty_texture_index]->Pixmap->Clear(Color(0, 0, 0, 0));
		m_textures[m_current_dirty_texture_index]->Texture = renderer->Create_Texture(m_textures[m_current_dirty_texture_index]->Pixmap, TextureFlags::LinearFilter);
	
		bool packed = m_textures[m_current_dirty_texture_index]->Packer.Pack(glyph_size, glyph_rect);
		DBG_ASSERT_STR(packed, "Could not pack glyph into brand new texture! To large?");
	}

	// Get glyph pixmap.
	Pixmap* glyph_pixmap = new Pixmap((unsigned char*)m_face->glyph->bitmap.buffer, m_face->glyph->bitmap.width, m_face->glyph->bitmap.rows, m_face->glyph->bitmap.width, PixmapFormat::R8, false);

	// Work out glyph texture position etc.
	int pixel_x	= glyph_rect.X;
	int pixel_y	= glyph_rect.Y;
	int pixel_w	= glyph_rect.Width;
	int pixel_h	= glyph_rect.Height;
		
	float uv_x	= (glyph_rect.X) / float(m_texture_size);
	float uv_y	= (glyph_rect.Y) / float(m_texture_size);
	float uv_w	= (glyph_rect.Width) / float(m_texture_size);		
	float uv_h	= (glyph_rect.Height) / float(m_texture_size);

	// Only render if we have a bitmap for this glyph.
	if (m_face->glyph->bitmap.buffer != NULL)
	{
		FreeType_FontTexture* texture = m_textures[m_current_dirty_texture_index];

#ifndef SDF_DOWNSCALE
		PixmapWindow window = glyph_pixmap->Window(Rect(0, 0, pixel_w, pixel_h), false, true);		
		texture->Pixmap->Paste(Point(pixel_x, pixel_y), window);
#endif
		
#ifdef SDF_DOWNSCALE
		Rect sdf_rect;
		sdf_rect.X		= Max(0, pixel_x - (m_glyph_spacing * 0.25));
		sdf_rect.Y		= Max(0, pixel_y - (m_glyph_spacing * 0.25));
		sdf_rect.Width	= Min(m_texture_size - sdf_rect.X, pixel_w + (m_glyph_spacing * 0.5)); 
		sdf_rect.Height	= Min(m_texture_size - sdf_rect.Y, pixel_h + (m_glyph_spacing * 0.5));
#else
		Rect sdf_rect;
		sdf_rect.X		= Max(0, pixel_x - m_sdf_spread);
		sdf_rect.Y		= Max(0, pixel_y - m_sdf_spread);
		sdf_rect.Width	= Min(m_texture_size - sdf_rect.X, pixel_w + (m_sdf_spread * 2)); 
		sdf_rect.Height	= Min(m_texture_size - sdf_rect.Y, pixel_h + (m_sdf_spread * 2));
#endif

#ifdef SDF_DOWNSCALE
		Generate_SDF(glyph_pixmap, texture->Pixmap, sdf_rect, (m_glyph_spacing * 0.25));
#else
		Generate_SDF(texture->Pixmap, sdf_rect);
#endif
		
		uv_x = (sdf_rect.X) / float(m_texture_size);
		uv_y = (sdf_rect.Y) / float(m_texture_size);
		uv_w = (sdf_rect.Width) / float(m_texture_size);		
		uv_h = (sdf_rect.Height) / float(m_texture_size);
	}

	// Add glyph to list.	
	FreeType_FontGlyph* new_glyph = new FreeType_FontGlyph();	
	new_glyph->FreeType_GlyphIndex = ft_glyph_index;
	new_glyph->Glyph.Texture	= m_textures[m_current_dirty_texture_index]->Texture;		
	new_glyph->Glyph.Glyph		= glyph;
	new_glyph->Glyph.UV			= Rect(uv_x, uv_y, uv_w, uv_h);
	new_glyph->TextureIndex		= m_current_dirty_texture_index;
	new_glyph->Glyph.Size		= Point(pixel_w, pixel_h);

#ifdef SDF_DOWNSCALE
	new_glyph->Glyph.Advance	= Point(((int)ceil(m_face->glyph->advance.x / m_sdf_downscale) >> 6), ((int)ceil(m_face->glyph->advance.y / m_sdf_downscale) >> 6));
	new_glyph->Glyph.Offset		= Point((m_face->glyph->bitmap_left / m_sdf_downscale), -ceil(m_face->glyph->bitmap_top / m_sdf_downscale));
#else
	new_glyph->Glyph.Advance	= Point((m_face->glyph->advance.x >> 6), (m_face->glyph->advance.y >> 6));
	new_glyph->Glyph.Offset		= Point(m_face->glyph->bitmap_left, -m_face->glyph->bitmap_top);
#endif

	if (abs(new_glyph->Glyph.Offset.Y) > m_baseline)
	{
		m_baseline = abs(new_glyph->Glyph.Offset.Y);
	}

	m_textures[m_current_dirty_texture_index]->Is_Dirty = true;

	m_glyphs.Set(glyph, new_glyph);

	// Release pixmap.
	SAFE_DELETE(glyph_pixmap);
}

#ifdef SDF_DOWNSCALE
void FreeType_Font::Generate_SDF(Pixmap* glyph_pixmap, Pixmap* pixmap, Rect rect, int offset)
#else
void FreeType_Font::Generate_SDF(Pixmap* pixmap, Rect rect)
#endif
{
#ifdef SDF_DOWNSCALE
	PixmapWindow glyph_window = glyph_pixmap->Window(Rect(0, 0, glyph_pixmap->Get_Width(), glyph_pixmap->Get_Height()), false, true);
#else
	PixmapWindow window = pixmap->Window(rect);
	Pixmap* result = new Pixmap(rect.Width, rect.Height, pixmap->Get_Format());
#endif

	//PixmapFactory::Save(".compiled/Data/Fonts/glyph.png", glyph_pixmap);
	
	for (int y = 0; y < rect.Height; y++)
	{
		for (int x = 0; x < rect.Width; x++)
		{
#ifdef SDF_DOWNSCALE
			float center_x = ((x - offset) * m_sdf_downscale) + (m_sdf_downscale * 0.5f);//(x + (m_sdf_downscale * 0.5f)) * m_sdf_downscale;
			float center_y = ((y - offset) * m_sdf_downscale) + (m_sdf_downscale * 0.5f);//(y + (m_sdf_downscale * 0.5f)) * m_sdf_downscale;
			float distance = Find_SDF_Distance(glyph_window, center_x, center_y);
#else
			float distance = Find_SDF_Distance(window, x, y);
#endif
			float alpha	= SDF_Distance_To_Alpha(distance);

		//	float small_sdf	= SDF_Distance_To_Alpha(small_distance);
		//	float big_sdf	= SDF_Distance_To_Alpha(big_distance);

#ifdef SDF_DOWNSCALE
			pixmap->Set_Pixel(rect.X + x, rect.Y + y, Color(alpha, alpha, alpha, alpha));
#else
			result->Set_Pixel(x, y, Color(alpha, alpha, alpha, alpha));
#endif
		//	if (distance < 0)
		//	{
		///		printf("%04d ", (int)distance);
		//	}
		//	else
		//	{
		//		printf("+%03d ", (int)distance);
		//	}
		}
		//printf("\n");
	}
	
#ifndef SDF_DOWNSCALE
	pixmap->Paste(Point(rect.X, rect.Y), result->Window(Rect(0, 0, rect.Width, rect.Height)));
	SAFE_DELETE(result);
#endif
}

unsigned char FreeType_Font::SDF_Distance_To_Alpha(float distance)
{
	float div = (distance / m_sdf_spread);
	float x = (0.5f * div);
	float alpha = 0.5f + x;
	alpha = Min(1.0f, Max(0.0, alpha)); 

	return (unsigned char)(alpha * 0xFF);
}

float FreeType_Font::Find_SDF_Distance(PixmapWindow window, int pixel_x, int pixel_y)
{	
	int width	= window.Get_Width();
	int height	= window.Get_Height();

	bool base_pixel = false;

	if (pixel_x >= 0 && pixel_y >= 0 && pixel_x < width && pixel_y < height)
	{
		base_pixel = window.Get_Pixel(pixel_x, pixel_y).R > m_sdf_threshold;
	}

	int delta = (int)ceilf(m_sdf_spread);
	float delta_max = (delta * delta + delta * delta) + 0.5f;
	float closest_distance = (delta * delta + delta * delta);
	
	bool found = false;

	for (int distance = 0; distance <= delta; distance++)
	{
		// Check outside rows/colums for this distance.
		for (int x = -distance; x <= distance; x++)
		{
			for (int y = -distance; y <= distance; y++)
			{
				if (!(x == -distance || x == distance || y == -distance || y == distance))
					continue;

				bool this_pixel = false;

				int glyph_x = pixel_x + x;
				int glyph_y = pixel_y + y;

				if (glyph_x >= 0 && glyph_y >= 0 && glyph_x < width && glyph_y < height)
				{
					this_pixel = window.Get_Pixel(glyph_x, glyph_y).R  > m_sdf_threshold;
				}

				if (base_pixel != this_pixel)
				{
					int square_dist = (x * x + y * y);

					if ((square_dist) <= delta_max && (square_dist) < (closest_distance))
					{
						closest_distance = square_dist;
						found = true;
					}
				}
			}
		}

		if (found == true)
		{
			break;
		}
	}

	float closest_dist = sqrtf(closest_distance);
	return (base_pixel ? 1 : -1) * Min(closest_dist, m_sdf_spread);
}
