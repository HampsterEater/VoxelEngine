// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_OPENGL_RENDERTARGET_
#define _ENGINE_RENDERER_OPENGL_RENDERTARGET_

#include "Engine\Renderer\Textures\RenderTarget.h"

#include "Engine\Renderer\OpenGL\OpenGL_Renderer.h"

#include <vector>

class OpenGL_Texture;

class OpenGL_RenderTarget : public RenderTarget
{
private:
	friend class OpenGL_Renderer;

	GLuint m_target_id;

	std::vector<OpenGL_Texture*>	m_color_textures;
	OpenGL_Texture*					m_depth_texture;
	OpenGL_Texture*					m_stencil_texture;

private:
	// Constructor!
	OpenGL_RenderTarget(GLuint target_id);

	GLuint Get_ID();

public:
		
	// Binding support.
	void Bind_Texture(RenderTargetBufferType::Type type, Texture* texture);
	void Validate();

	// Destructor!
	~OpenGL_RenderTarget();

};

#endif

