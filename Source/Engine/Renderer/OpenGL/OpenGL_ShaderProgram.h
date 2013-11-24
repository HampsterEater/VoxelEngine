// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_OPENGL_SHADERPROGRAM_
#define _ENGINE_RENDERER_OPENGL_SHADERPROGRAM_

#include "Engine\Renderer\Shaders\ShaderProgram.h"

#include "Engine\Renderer\OpenGL\OpenGL_Renderer.h"

class OpenGL_ShaderProgram : public ShaderProgram
{
private:
	friend class OpenGL_Renderer;

	GLuint m_id;
	std::vector<Shader*> m_shaders;

private:
	// Constructor!
	OpenGL_ShaderProgram(GLuint id, std::vector<Shader*>& shaders);

public:

	// Destructor!
	~OpenGL_ShaderProgram();

	// Variable binding.
	void Bind_Texture	(const char* name, int texture_index);
	void Bind_Matrix	(const char* name, Matrix4 matrix);
	void Bind_Vector	(const char* name, Vector3 vector);
	void Bind_Vector	(const char* name, Vector4 vector);
	void Bind_Float		(const char* name, float value);
	void Bind_Int		(const char* name, int value);
	void Bind_Bool		(const char* name, bool value);

	GLuint Get_ID();

};

#endif

