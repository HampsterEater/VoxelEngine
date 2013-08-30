// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// This is a special file, it is included by default in every other file
// so be careful what you put in here! It's mainly just for definitions
// of things that should be globally available (very little!).

#ifndef NULL
	#define NULL 0
#endif

#ifdef _WIN32
	#define PLATFORM_WIN32
#else
	#error "Unsupported platform."
#endif

#ifdef NDEBUG
#define RELEASE_BUILD
#else
#define DEBUG_BUILD
#endif

// Libraries always included for each platform.
#ifdef PLATFORM_WIN32
#include <stdio.h>
#include <algorithm>
#endif

// Debugging macros!
#ifdef PLATFORM_WIN32
	#define DBG_ASSERT(cond) \
		if (!(cond)) \
		{ \
			printf("====== ASSERT FAILED ======\n"); \
			printf("%s:%i\n", __FILE__, __LINE__); \
			printf("%s\n", #cond); \
			__debugbreak(); \
		}

	#define DBG_LOG(format, ...) \
		printf(format "\n", __VA_ARGS__); 
#else
	#error "Unsupported platform."
#endif

// Memory management whowhar.
#define SAFE_DELETE(x) \
		if ((x)) \
		{ \
			delete x; \
			x = NULL; \
		} 
#define SAFE_DELETE_ARRAY(x) \
		if ((x)) \
		{ \
			delete[] x; \
			x = NULL; \
		} 

// Include memory function overrides.
#include "Generic\Memory\Memory.h"
