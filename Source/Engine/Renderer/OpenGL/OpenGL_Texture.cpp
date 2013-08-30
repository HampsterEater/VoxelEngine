// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\OpenGL\OpenGL_Texture.h"

OpenGL_Texture::OpenGL_Texture(GLuint texture_id, char* data, int width, int height, int pitch, TextureFormat::Type format)
	: Texture(data, width, height, pitch, format)
	, m_texture_id(texture_id)
{

}

GLuint OpenGL_Texture::Get_ID() 
{ 
	return m_texture_id; 
}

OpenGL_Texture::~OpenGL_Texture()
{
	glDeleteTextures(1, &m_texture_id);
}
