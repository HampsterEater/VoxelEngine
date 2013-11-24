// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_TEXTURE_
#define _ENGINE_RENDERER_TEXTURES_TEXTURE_

#include "Engine\Renderer\Textures\PixmapFormat.h"
#include "Engine\Renderer\Textures\TextureFormat.h"

class Pixmap;

struct TextureFlags
{
	enum Type
	{
		None = 0,
		AllowRepeat = 1,
		LinearFilter = 2
	};
};

class Texture
{
protected:
	Pixmap*				m_pixmap;
	int					m_width;
	int					m_height;
	int					m_pitch;
	TextureFormat::Type m_format;
	
protected:

	// Constructor!
	Texture(int width, int height, int pitch, TextureFormat::Type format);
	Texture(Pixmap* pixmap);

public:

	// Destructor!
	virtual ~Texture();

	// Members that have to be overidden.
	Pixmap*							Get_Pixmap	 () const;

	// Note: When setting data ownership of the pixmap is transfered to the texture 
	//		 object which will deal with deallocation.
	virtual void					Set_Pixmap	 (Pixmap* pixmap) = 0;

	int								Get_Width	 () const;
	int								Get_Height	 () const;
	int								Get_Pitch	 () const;
	TextureFormat::Type				Get_Format	 () const;
};

#endif

