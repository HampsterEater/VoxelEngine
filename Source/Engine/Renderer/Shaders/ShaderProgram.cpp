// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Shaders\ShaderProgram.h"
#include "Engine\Renderer\Renderer.h"

#include <vector>

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::~ShaderProgram()
{
}

ShaderProgram* ShaderProgram::Create(Shader* s1)
{
	std::vector<Shader*> shaders;
	shaders.push_back(s1);

	return Renderer::Get()->Create_Shader_Program(shaders);
}

ShaderProgram* ShaderProgram::Create(Shader* s1, Shader* s2)
{
	std::vector<Shader*> shaders;
	shaders.push_back(s1);
	shaders.push_back(s2);

	return Renderer::Get()->Create_Shader_Program(shaders);
}

ShaderProgram* ShaderProgram::Create(Shader* s1, Shader* s2, Shader* s3)
{
	std::vector<Shader*> shaders;
	shaders.push_back(s1);
	shaders.push_back(s2);
	shaders.push_back(s3);

	return Renderer::Get()->Create_Shader_Program(shaders);
}

ShaderProgram* ShaderProgram::Create(Shader* s1, Shader* s2, Shader* s3, Shader* s4)
{
	std::vector<Shader*> shaders;
	shaders.push_back(s1);
	shaders.push_back(s2);
	shaders.push_back(s3);
	shaders.push_back(s4);

	return Renderer::Get()->Create_Shader_Program(shaders);
}


