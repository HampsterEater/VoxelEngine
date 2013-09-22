// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\OpenGL\OpenGL_Shader.h"
#include "Engine\Renderer\OpenGL\OpenGL_Renderer.h"

OpenGL_Shader::OpenGL_Shader(GLuint id, char* source, ShaderType::Type type)
	: Shader(source, type)
	, m_id(id)
{
}

OpenGL_Shader::~OpenGL_Shader()
{
	glDeleteShader(m_id);
}

GLuint OpenGL_Shader::Get_ID()
{
	return m_id;
}
