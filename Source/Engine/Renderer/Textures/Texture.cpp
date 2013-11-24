// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Textures\Texture.h"
#include "Engine\Renderer\Textures\Pixmap.h"

Texture::Texture(int width, int height, int pitch, TextureFormat::Type format)
	: m_width(width)
	, m_height(height)
	, m_pitch(pitch)
	, m_format(format)
	, m_pixmap(NULL)
{
}

Texture::Texture(Pixmap* pixmap)
	: m_width(pixmap->Get_Width())
	, m_height(pixmap->Get_Height())
	, m_pitch(pixmap->Get_Pitch())
	, m_format(pixmap->Get_Texture_Format())
	, m_pixmap(pixmap)
{
}

Texture::~Texture()
{
	SAFE_DELETE(m_pixmap);
}

int	Texture::Get_Width() const
{
	return m_width;
}

int	Texture::Get_Height() const
{
	return m_height;
}

int	Texture::Get_Pitch() const
{
	return m_pitch;
}

TextureFormat::Type	Texture::Get_Format() const
{
	return m_format;
}

Pixmap*	Texture::Get_Pixmap() const
{
	return m_pixmap;
}
