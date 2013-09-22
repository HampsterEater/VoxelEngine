// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_TICKABLE_
#define _ENGINE_TICKABLE_

#include "Engine\Engine\FrameTime.h"

#include "Generic\Types\Vector3.h"

class Tickable
{
protected:

public:

	// Base functions.
	virtual void Tick(const FrameTime& time) = 0;

};

#endif

