// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_PIXMAP_
#define _ENGINE_RENDERER_TEXTURES_PIXMAP_

#include "Generic\Types\Color.h"
#include "Generic\Types\Rectangle.h"
#include "Engine\Renderer\Textures\TextureFormat.h"
#include "Engine\Renderer\Textures\PixmapFormat.h"

class Pixmap;

// Used to represent a specific rectangle of a pixmap. Only valid for
// as long as the source pixmap exists.
struct PixmapWindow
{
private:
	Pixmap* Pixmap;
	Rect	Rect;
	bool	InvertX;
	bool	InvertY;

	friend class Pixmap;

public:
	Color Get_Pixel(int x, int y);

	int Get_Width()  { return Rect.Width; }
	int Get_Height() { return Rect.Height; }

};

// Pixmaps are simply "maps" of pixels. They simply contain a simple 
// interface for storing and manipulating pixel data directly. They are
// primarily used as an intermediary format between loading texture data
// from files and converting the data into hardware textures.
class Pixmap
{
private:
	unsigned char* 
		m_data;

	int   
		m_width,
		m_height,
		m_pitch,
		m_bpp;

	PixmapFormat::Type
		m_format;

	TextureFormat::Type
		m_texture_format;

	bool
		m_owns_data;

public:
	Pixmap(int width, int height, PixmapFormat::Type format);

	// Data used to intialize pixmaps is from that point on "owned" by the pixmap and will
	// be disposed of when the pixmap is.
	Pixmap(unsigned char* data, int width, int height, int pitch, PixmapFormat::Type format, bool owns_data = true);	

	~Pixmap();

	unsigned char* Get_Data();
	int Get_Width();
	int Get_Pitch();
	int Get_Height();
	PixmapFormat::Type Get_Format();
	TextureFormat::Type Get_Texture_Format();

	void  Clear(Color c);

	void  Set_Pixel(int x, int y, Color c);
	Color Get_Pixel(int x, int y);

	PixmapWindow Window(Rect rect, bool invert_x = false, bool invert_y = false);
	void Paste(Point position, PixmapWindow rect);

};

#endif