// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_PLATFORM_WIN32_PLATFORM_
#define _ENGINE_PLATFORM_WIN32_PLATFORM_

#include "Generic\Patterns\Singleton.h"
#include "Engine\Platform\Platform.h"

class Win32_Platform : public Platform
{
public:

	// Time functions.
	float Get_Ticks();

private:
	friend class Platform;

	Win32_Platform();

};

#endif

