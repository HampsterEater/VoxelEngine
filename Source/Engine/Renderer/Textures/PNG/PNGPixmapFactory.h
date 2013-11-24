// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_PNG_PIXMAPFACTORY_
#define _ENGINE_RENDERER_TEXTURES_PNG_PIXMAPFACTORY_

#include "Engine\Renderer\Textures\Pixmap.h"
#include "Engine\Renderer\Textures\PixmapFactory.h"

#include "libpng\png.h"

class PNGPixmapFactory : public PixmapFactory
{
private:
	static void libpng_read_function(png_structp pngPtr, png_bytep data, png_size_t length);
	static void libpng_write_function(png_structp pngPtr, png_bytep data, png_size_t length);

public:
	Pixmap*  Try_Load(const char* url);
	bool	 Try_Save(const char* url, Pixmap* texture);

};

#endif

