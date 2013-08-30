// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Renderer.h"

#include "Generic\Patterns\Singleton.h"

#ifdef PLATFORM_WIN32
#include "Engine\Renderer\OpenGL\OpenGL_Renderer.h"
#endif

Renderer* Renderer::Create()
{
#ifdef PLATFORM_WIN32
	return new OpenGL_Renderer();
#else
	#error "Platform unsupported."
#endif
}

