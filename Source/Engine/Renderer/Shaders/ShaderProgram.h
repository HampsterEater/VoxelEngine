// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_SHADERS_SHADERPROGRAM_
#define _ENGINE_RENDERER_SHADERS_SHADERPROGRAM_

#include "Generic\Types\Matrix4.h"
#include "Generic\Types\Vector3.h"
#include "Generic\Types\Vector4.h"

#include "Engine\Renderer\Shaders\Shader.h"

class Texture;

class ShaderProgram
{
private:

protected:

	// Constructor!
	ShaderProgram();

public:

	// Destructor!
	virtual ~ShaderProgram();
	
	// Variable binding.
	virtual void Bind_Texture	(const char* name, int texture_index) = 0;
	virtual void Bind_Matrix	(const char* name, Matrix4 matrix) = 0;
	virtual void Bind_Vector	(const char* name, Vector3 vector) = 0;
	virtual void Bind_Vector	(const char* name, Vector4 vector) = 0;
	virtual void Bind_Float		(const char* name, float value) = 0;
	virtual void Bind_Int		(const char* name, int value) = 0;
	virtual void Bind_Bool		(const char* name, bool value) = 0;

	// Create shader programs.
	static ShaderProgram* Create(Shader* s1);
	static ShaderProgram* Create(Shader* s1, Shader* s2);
	static ShaderProgram* Create(Shader* s1, Shader* s2, Shader* s3);
	static ShaderProgram* Create(Shader* s1, Shader* s2, Shader* s3, Shader* s4);

};

#endif

