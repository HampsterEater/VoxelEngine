// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Audio\AudioRenderer.h"

#ifdef PLATFORM_WIN32
#include "Engine\Audio\FMod\FMod_AudioRenderer.h"
#endif

AudioRenderer* AudioRenderer::Create()
{
#ifdef PLATFORM_WIN32
	return new FMod_AudioRenderer();
#else
	#error "Platform unsupported."
#endif
}