// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Display\Display.h"

#include "Generic\Patterns\Singleton.h"

#ifdef PLATFORM_WIN32
#include "Engine\Display\Win32\Win32_Display.h"
#endif

Display* Display::Create(const char* title, int width, int height, bool fullscreen)
{
#ifdef PLATFORM_WIN32
	return new Win32_Display(title, width, height, fullscreen);
#else
	#error "Platform unsupported."
#endif
}

