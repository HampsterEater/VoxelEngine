// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Input\Input.h"

#ifdef PLATFORM_WIN32
#include "Engine\Input\Win32\Win32_Input.h"
#endif

Input* Input::Create()
{
#ifdef PLATFORM_WIN32
	return new Win32_Input();
#endif
}
	
