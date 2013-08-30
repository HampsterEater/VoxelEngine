// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Platform\Platform.h"

#include "Generic\Patterns\Singleton.h"

#ifdef PLATFORM_WIN32
#include "Engine\Platform\Win32\Win32_Platform.h"
#endif

Platform* Platform::Create()
{
#ifdef PLATFORM_WIN32
	return new Win32_Platform();
#else
	#error "Platform unsupported."
#endif
}

