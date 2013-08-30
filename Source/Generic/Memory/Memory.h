// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MEMORY_
#define _GENERIC_MEMORY_

// Required implementations.
void* platform_malloc(std::size_t size);
void  platform_free  (void* ptr);

// Overrides
void* operator new(std::size_t count);
void* operator new[](std::size_t count);
void* operator new(std::size_t count, const std::nothrow_t&);
void* operator new[](std::size_t count, const std::nothrow_t&);
void operator delete(void* ptr);
void operator delete[](void* ptr);
void operator delete(void* ptr, const std::nothrow_t&);
void operator delete[](void* ptr, const std::nothrow_t&);

// Include header for specific platform.
#ifdef PLATFORM_WIN32
#include "Generic\Memory\Win32\Win32_Memory.h"
#else
#error Unsupported platform.
#endif

#endif
