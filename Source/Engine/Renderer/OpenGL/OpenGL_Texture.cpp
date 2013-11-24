// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\OpenGL\OpenGL_Texture.h"
#include "Engine\Renderer\Textures\Pixmap.h"

OpenGL_Texture::OpenGL_Texture(GLuint texture_id, int width, int height, int pitch, TextureFormat::Type format)
	: Texture(width, height, pitch, format)
	, m_texture_id(texture_id)
{

}

OpenGL_Texture::OpenGL_Texture(GLuint texture_id, Pixmap* pixmap)
	: Texture(pixmap)
	, m_texture_id(texture_id)
{

}

GLuint OpenGL_Texture::Get_ID() const
{ 
	return m_texture_id; 
}

OpenGL_Texture::~OpenGL_Texture()
{
	glDeleteTextures(1, &m_texture_id);
}

void OpenGL_Texture::Set_Pixmap(Pixmap* pixmap)
{
	// Generate and bind texture.
	GLint original_bind = 0;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &original_bind);
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, m_texture_id);
	
	m_format			= pixmap->Get_Texture_Format();
	m_width				= pixmap->Get_Width();
	m_height			= pixmap->Get_Height();
	unsigned char* data	= pixmap->Get_Data();

	// Upload data to GPU.
	switch (m_format)
	{
	case TextureFormat::R8G8B8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		}
	case TextureFormat::R32FG32FB32FA32F:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, data);
			break;
		}
	case TextureFormat::R8G8B8A8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			break;
		}
	case TextureFormat::DepthFormat:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
			break;
		}
	case TextureFormat::StencilFormat:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, m_width, m_height, 0, GL_STENCIL_INDEX, GL_FLOAT, data);
			break;
		}
	case TextureFormat::Luminosity:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
			break;
		}

	default:
		{
			// Format not supported.
			DBG_ASSERT(false);
			break;
		}
	}
	
	glBindTexture(GL_TEXTURE_2D, original_bind);

	// Replace current buffer with new one.
	if (pixmap != m_pixmap)
	{
		SAFE_DELETE(m_pixmap);
		m_pixmap = pixmap;
	}
}

