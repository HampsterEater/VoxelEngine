// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_OPENGL_SHADER_
#define _ENGINE_RENDERER_OPENGL_SHADER_

#include "Engine\Renderer\Shaders\Shader.h"

class Shader;
class OpenGL_Renderer;

#include <Windows.h>
#include <gl/GL.h>

class OpenGL_Shader : public Shader
{
private:
	friend class OpenGL_Renderer;

	GLuint m_id;

private:
	// Constructor!
	OpenGL_Shader(GLuint id, char* source, ShaderType::Type type);

public:

	// Destructor!
	~OpenGL_Shader();

	GLuint Get_ID();

};

#endif

