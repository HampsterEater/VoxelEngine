// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Memory\Memory.h"

void* operator new(std::size_t count)
{
	return platform_malloc(count);
}

void* operator new[](std::size_t count)
{
	return platform_malloc(count);
}

void* operator new(std::size_t count, const std::nothrow_t&)
{
	return platform_malloc(count);
}

void* operator new[](std::size_t count, const std::nothrow_t&)
{
	return platform_malloc(count);
}
	
void operator delete(void* ptr)
{
	return platform_free(ptr);
}

void operator delete[](void* ptr)
{
	return platform_free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&)
{
	return platform_free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&)
{
	return platform_free(ptr);
}