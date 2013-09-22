// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\OpenGL\OpenGL_RenderTarget.h"
#include "Engine\Renderer\OpenGL\OpenGL_Texture.h"

OpenGL_RenderTarget::OpenGL_RenderTarget(GLuint target_id) 
	: RenderTarget()
	, m_target_id(target_id)
	, m_depth_texture(NULL)
	, m_stencil_texture(NULL)
{
}
	
OpenGL_RenderTarget::~OpenGL_RenderTarget()
{
}

GLuint OpenGL_RenderTarget::Get_ID()
{
	return m_target_id;
}

void OpenGL_RenderTarget::Bind_Texture(RenderTargetBufferType::Type type, Texture* texture)
{
	OpenGL_Texture* glTexture = dynamic_cast<OpenGL_Texture*>(texture);
	DBG_ASSERT(glTexture != NULL);

	Renderer::Get()->Bind_Render_Target(this);

	switch (type)
	{
	case RenderTargetBufferType::Color:
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + m_color_textures.size(), GL_TEXTURE_2D, glTexture->Get_ID(), 0);

	       // GLenum DrawBuffers[2] = {GL_BACK_RIGHT, GL_COLOR_ATTACHMENT0};
		   // glDrawBuffers(2, DrawBuffers); 

			m_color_textures.push_back(glTexture);
			break;
		}
	case RenderTargetBufferType::Depth:
		{
			DBG_ASSERT(m_depth_texture == NULL);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, glTexture->Get_ID(), 0);

			m_depth_texture = glTexture;
			break;
		}
	case RenderTargetBufferType::Stencil:
		{
			DBG_ASSERT(m_stencil_texture == NULL);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, glTexture->Get_ID(), 0);

			m_stencil_texture = glTexture;
			break;
		}
	default:
		{
			// Format not supported.
			DBG_ASSERT(false);
			break;
		}
	}
}

void OpenGL_RenderTarget::Validate()
{
	Renderer::Get()->Bind_Render_Target(this);

	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	DBG_ASSERT(status == GL_FRAMEBUFFER_COMPLETE);
}

