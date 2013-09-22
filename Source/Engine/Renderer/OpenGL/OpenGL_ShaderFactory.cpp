// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Shaders\Shader.h"
#include "Engine\Renderer\OpenGL\OpenGL_ShaderFactory.h"

#include "Engine\Renderer\Renderer.h"

#include "Engine\IO\StreamFactory.h"

#include <string>

Shader* OpenGL_ShaderFactory::Try_Load(const char* url, ShaderType::Type type)
{
	Stream* stream = StreamFactory::Open(url, StreamMode::Read);
	if (stream == NULL)
	{
		return NULL;
	}

	int   length = stream->Length();
	char* data	 = new char[length + 1];

	stream->Read(data, 0, length);
	data[length] = '\0';

	Shader* shader = Renderer::Get()->Create_Shader(data, type);
	if (shader == NULL)
	{
		SAFE_DELETE_ARRAY(data);
		return NULL;
	}

	return shader;
}