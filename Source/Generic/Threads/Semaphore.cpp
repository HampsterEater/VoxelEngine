// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Threads\Semaphore.h"

#ifdef PLATFORM_WIN32
#include "Generic\Threads\Win32\Win32_Semaphore.h"
#endif

Semaphore* Semaphore::Create()
{
#ifdef PLATFORM_WIN32
	return new Win32_Semaphore();
#else
	#error "Platform unsupported."
#endif
}

