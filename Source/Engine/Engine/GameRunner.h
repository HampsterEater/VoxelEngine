// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_GAMERUNNER_
#define _ENGINE_GAMERUNNER_

#include "Engine\Engine\FrameTime.h"

class GameEngine;

class GameRunner
{
private:

protected:

	friend class GameEngine;

	// Base functions.
	virtual void Start() = 0;
	virtual void End() = 0;
	virtual void Tick(const FrameTime& time) = 0;

};

#endif

