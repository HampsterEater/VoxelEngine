// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FREETYPE_FONT_
#define _ENGINE_RENDERER_TEXT_FREETYPE_FONT_

#include "Generic\Types\Rectangle.h"
#include "Generic\Math\RectanglePacker.h"

#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Textures\Pixmap.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class ConfigFile;
class Pixmap;

#define SDF_DOWNSCALE

struct FreeType_FontGlyph
{
public:
	unsigned int	FreeType_GlyphIndex;
	int				TextureIndex;
	FontGlyph		Glyph;
};

struct FreeType_FontTexture
{
public:
	Texture*		Texture;
	Pixmap*			Pixmap;
	bool			Is_Dirty;
	RectanglePacker	Packer;
};

class FreeType_Font : public Font
{
private:
	FT_Library										m_library;
	FT_Face											m_face;

	char*											m_buffer;

	FreeType_FontTexture**							m_textures;
	HashTable<FreeType_FontGlyph*, unsigned int>	m_glyphs;
	int												m_current_dirty_texture_index;

	int												m_sdf_spread;
	int												m_sdf_downscale;
	int												m_sdf_threshold;
	int												m_texture_size;
	int												m_max_textures;
	int												m_glyph_size;
	int												m_glyph_spacing;

	bool											m_textures_locked;

	int												m_baseline;

public:

	// Destructor!
	FreeType_Font(FT_Library library, FT_Face face, char* buffer, ConfigFile* config);
	FreeType_Font(FT_Library library, FT_Face face, char* buffer);
	~FreeType_Font();

	// FreeType stuff.
	void Lock_Textures();
	FreeType_FontTexture** Get_Textures(int& texture_count);
	void Unlock_Textures();
	void Add_Glyphs(const char* glyphs);
	void Add_Glyph(unsigned int glyph);
	HashTable<FreeType_FontGlyph*, unsigned int>& Get_Glyphs();
	bool Load_Compiled_Config(ConfigFile* config);

#ifdef SDF_DOWNSCALE
	void Generate_SDF(Pixmap* glyph_pixmap, Pixmap* pixmap, Rect rect, int offset);
#else
	void Generate_SDF(Pixmap* pixmap, Rect rect);
#endif
	float Find_SDF_Distance(PixmapWindow window, int pixel_x, int pixel_y);
	unsigned char SDF_Distance_To_Alpha(float distance);
	float Get_Baseline();
	
	//void Generate_SDF(Pixmap* pixmap, Rect rect);

	// Actual font stuff!	
	float Get_SDF_Spread();
	float Get_SDF_Source_Size();
	FontGlyph Get_Glyph(unsigned int character);
	Point	  Get_Kerning(unsigned int prev, unsigned int next);

};

#endif

