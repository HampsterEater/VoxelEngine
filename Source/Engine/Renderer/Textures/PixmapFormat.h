// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_PIXMAPFORMAT_
#define _ENGINE_RENDERER_TEXTURES_PIXMAPFORMAT_

struct PixmapFormat
{
	// If you modify this, make sure to update the BPPForPixmapFormat array
	// in the cpp file.
	enum Type
	{
		R8 = 0,
		R8G8B8A8,
		R8G8B8
	};
};

#endif


