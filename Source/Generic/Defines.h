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
	#define DBG_ASSERT_STR(cond, format, ...) \
		if (!(cond)) \
		{ \
			printf("====== ASSERT FAILED ======\n"); \
			printf("%s:%i\n", __FILE__, __LINE__); \
			printf("%s\n", #cond); \
			printf("Message: " format "\n", __VA_ARGS__); \
			__debugbreak(); \
		}

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
	//"[" __FUNCTION__  ":%i] " 
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

// Dammnit windows, this is cross platform code, I don't care about your
// secure functions.
#if defined(_MSC_VER)
	#ifndef _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_WARNINGS (1)
	#endif
	#ifndef _CRT_SECURE_NO_DEPRECATE
		#define _CRT_SECURE_NO_DEPRECATE (1)
	#endif
	#pragma warning(disable : 4996)
#endif

// Special compiler keywords.
#if defined(_MSC_VER)	
	#define INLINE __forceinline
#else
	#error "Unsupported platform."
#endif

// Special types.
#define s8  char
#define u8  unsigned char
#define s16 short
#define u16 unsigned short
#define s32 int
#define u32 unsigned int
#define s64 long long int
#define u64 unsigned long long int

// Include memory function overrides.
#include "Generic\Memory\Memory.h"
