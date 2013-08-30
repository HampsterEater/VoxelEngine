// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Threads\Mutex.h"

#ifdef PLATFORM_WIN32
#include "Generic\Threads\Win32\Win32_Mutex.h"
#endif

Mutex* Mutex::Create()
{
#ifdef PLATFORM_WIN32
	return new Win32_Mutex();
#else
	#error "Platform unsupported."
#endif
}

