// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_TEXTUREFORMAT_
#define _ENGINE_RENDERER_TEXTURES_TEXTUREFORMAT_

struct TextureFormat
{
	enum Type
	{
		R8G8B8A8,
		R8G8B8,
		R32FG32FB32FA32F,
		DepthFormat,
		StencilFormat,
		Luminosity
	};
};

#endif

