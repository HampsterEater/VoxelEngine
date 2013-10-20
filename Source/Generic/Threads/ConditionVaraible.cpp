// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Threads\ConditionVariable.h"

#ifdef PLATFORM_WIN32
#include "Generic\Threads\Win32\Win32_ConditionVariable.h"
#endif

ConditionVariable* ConditionVariable::Create()
{
#ifdef PLATFORM_WIN32
	return new Win32_ConditionVariable();
#else
	#error "Platform unsupported."
#endif
}

