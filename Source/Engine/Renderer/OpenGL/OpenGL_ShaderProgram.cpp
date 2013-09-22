// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\OpenGL\OpenGL_ShaderProgram.h"

OpenGL_ShaderProgram::OpenGL_ShaderProgram(GLuint id, std::vector<Shader*>& shaders)
	: m_id(id)
	, m_shaders(shaders)
{
}

OpenGL_ShaderProgram::~OpenGL_ShaderProgram()
{
	glDeleteProgram(m_id);
}

GLuint OpenGL_ShaderProgram::Get_ID()
{
	return m_id;
}

void OpenGL_ShaderProgram::Bind_Texture(const char* name, int texture_index)
{
	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = glGetUniformLocation(m_id, name);

	if (image_id >= 0)
	{
		glUniform1i(image_id, texture_index); 
		//DBG_LOG("Bound texture %s[%i] to %i\n", name, image_id, texture_index);
	}
//	else
//	{
//		DBG_LOG("Failed to bind texture %s\n", name);
//	}
}

void OpenGL_ShaderProgram::Bind_Int(const char* name, int val)
{
	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = glGetUniformLocation(m_id, name);

	if (image_id >= 0)
	{
		glUniform1i(image_id, val); 
	}
//	else
//	{
//		DBG_LOG("Failed to bind int %s\n", name);
//	}
}

void OpenGL_ShaderProgram::Bind_Matrix(const char* name, Matrix4 matrix)
{
	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = glGetUniformLocation(m_id, name);
	
	if (image_id >= 0)
	{
		glUniformMatrix4fv(image_id, 1, GL_FALSE, matrix.Elements);
	}
//	else
//	{
//		DBG_LOG("Failed to bind matrix %s\n", name);
//	}
}

void OpenGL_ShaderProgram::Bind_Vector(const char* name, Vector3 vector)
{
	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = glGetUniformLocation(m_id, name);
	
	if (image_id >= 0)
	{
		glUniform3f(image_id, vector.X, vector.Y, vector.Z);
	}
//	else
//	{
//		DBG_LOG("Failed to bind vector3 %s\n", name);
//	}
}

void OpenGL_ShaderProgram::Bind_Vector(const char* name, Vector4 vector)
{
	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = glGetUniformLocation(m_id, name);
	
	if (image_id >= 0)
	{
		glUniform4f(image_id, vector.X, vector.Y, vector.Z, vector.W);
	}
//	else
//	{
//		DBG_LOG("Failed to bind vector4 %s\n", name);
//	}
}

void OpenGL_ShaderProgram::Bind_Float(const char* name, float value)
{
	Renderer::Get()->Bind_Shader_Program(this);

	GLint image_id = glGetUniformLocation(m_id, name);
	
	if (image_id >= 0)
	{
		glUniform1f(image_id, value);
	}
//	else
//	{
//		DBG_LOG("Failed to bind float %s\n", name);
//	}
}