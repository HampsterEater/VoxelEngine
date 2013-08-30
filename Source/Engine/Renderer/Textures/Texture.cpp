// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Textures\Texture.h"

Texture::Texture(char* data, int width, int height, int pitch, TextureFormat::Type format)
	: m_data(data)
	, m_width(width)
	, m_height(height)
	, m_pitch(pitch)
	, m_format(format)
{
}

Texture::~Texture()
{
	SAFE_DELETE(m_data);
}

int	Texture::Get_Width()
{
	return m_width;
}

int	Texture::Get_Height()
{
	return m_height;
}

int	Texture::Get_Pitch()
{
	return m_pitch;
}

const char* Texture::Get_Data()
{
	return m_data;
}

TextureFormat::Type	Texture::Get_Format()
{
	return m_format;
}
