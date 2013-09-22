// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_SHADERS_OPENGL_SHADERFACTORY_
#define _ENGINE_RENDERER_SHADERS_OPENGL_SHADERFACTORY_

#include "Engine\Renderer\Shaders\ShaderFactory.h"

#include "libpng\png.h"

class OpenGL_ShaderFactory : public ShaderFactory
{
private:

public:
	Shader* Try_Load(const char* url, ShaderType::Type type);

};

#endif

