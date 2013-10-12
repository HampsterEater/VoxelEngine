// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FREETYPE_FONT_
#define _ENGINE_RENDERER_TEXT_FREETYPE_FONT_

#include "Generic\Types\Rectangle.h"
#include "Generic\Math\RectanglePacker.h"

#include "Engine\Renderer\Text\Font.h"

#include <ft2build.h>
#include FT_FREETYPE_H

//#define MAX_TEXTURES				8
//#define TEXTURE_SIZE				512
//#define GLYPH_SIZE					32
//#define GLYPH_SPACING				2
//#define GLYPH_SIGNED_SPREAD			32
//#define GLYPH_SIGNED_THRESHOLD		128
//#define GLYPH_DOWNSCALE_FACTOR		32

class ConfigFile;

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
	unsigned char*	Buffer;
	bool			Is_Dirty;
	RectanglePacker	Packer;
};

class FreeType_Font : public Font
{
private:
	FT_Library										m_library;
	FT_Face											m_face;

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

public:

	// Destructor!
	FreeType_Font(FT_Library library, FT_Face face, char* buffer, ConfigFile* config);
	~FreeType_Font();

	// FreeType stuff.
	void Lock_Textures();
	FreeType_FontTexture** Get_Textures(int& texture_count);
	void Unlock_Textures();
	void Add_Glyphs(const char* glyphs);
	void Add_Glyph(unsigned int glyph);
	HashTable<FreeType_FontGlyph*, unsigned int>& Get_Glyphs();

	float Find_SDF_Distance(int x, int y);
	unsigned char SDF_Distance_To_Alpha(float distance);
	float Get_Baseline();

	// Actual font stuff!	
	float Get_SDF_Spread();
	float Get_SDF_Source_Size();
	FontGlyph Get_Glyph(unsigned int character);
	Point	  Get_Kerning(unsigned int prev, unsigned int next);

};

#endif

