// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_PLATFORM_
#define _ENGINE_PLATFORM_

#include "Generic\Patterns\Singleton.h"

class Platform : public Singleton<Platform>
{
public:
	static Platform* Create();

	// Time functions.
	virtual float Get_Ticks() = 0;
};

#endif

