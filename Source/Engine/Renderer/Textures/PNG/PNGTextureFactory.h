// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_PNG_TEXTUREFACTORY_
#define _ENGINE_RENDERER_TEXTURES_PNG_TEXTUREFACTORY_

#include "Engine\Renderer\Textures\TextureFactory.h"

#include "libpng\png.h"

class PNGTextureFactory : public TextureFactory
{
private:
	static void libpng_read_function(png_structp pngPtr, png_bytep data, png_size_t length);

public:
	Texture* Try_Load(const char* url, TextureFlags::Type flags);

};

#endif

