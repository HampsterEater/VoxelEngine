// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Memory\Win32\Win32_Memory.h"

#include <Windows.h>

void* platform_malloc(std::size_t size)
{
	return GlobalAlloc(GMEM_FIXED, size);
}

void platform_free(void* ptr)
{
	GlobalFree(ptr);
}
