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

// Architecture types.
#if defined(__i386__) || defined(_M_IX86) || defined(_X86_)
	#define PLATFORM_32BIT
    #define PLATFORM_X86     
#elif defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
	#define PLATFORM_64BIT
    #define PLATFORM_X64     
    #define PLATFORM_AMD64   
#elif defined(__amd64__) || defined(_M_IA64)
	#define PLATFORM_64BIT
    #define PLATFORM_ITANIUM64   
#elif defined(_M_PPC)
	#define PLATFORM_32BIT
	#define PLATFORM_PPC		
#else
	#error "Unsupported platform."
#endif

// Endianness defines
#if defined(_M_PPCBE) || defined(__BIG_ENDIAN__) || defined(_BIG_ENDIAN)
    #define PLATFORM_BIG_ENDIAN  1
#elif defined(_M_PPC) || defined(__LITTLE_ENDIAN__) || defined(_LITTLE_ENDIAN)
    #define PLATFORM_LITTLE_ENDIAN   1
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == _LITTLE_ENDIAN
    #define PLATFORM_LITTLE_ENDIAN   1
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == _BIG_ENDIAN
    #define PLATFORM_BIG_ENDIAN      1
#elif defined(__sparc) || defined(__sparc__) \
    || defined(_POWER) || defined(__powerpc__) \
    || defined(__ppc__) || defined(__hpux) \
    || defined(_MIPSEB) || defined(_POWER) \
    || defined(__s390__)
    #define PLATFORM_BIG_ENDIAN      1
#elif defined(__i386__) || defined(__alpha__) \
    || defined(__ia64) || defined(__ia64__) \
    || defined(_M_IX86) || defined(_M_IA64) \
    || defined(_M_ALPHA) || defined(__amd64) \
    || defined(__amd64__) || defined(_M_AMD64) \
    || defined(__x86_64) || defined(__x86_64__) \
    || defined(_M_X64) || defined(__bfin__)
    #define PLATFORM_LITTLE_ENDIAN   1
#else
	#error "Unsupported platform."
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
