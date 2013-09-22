// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_GAMERUNNER_
#define _ENGINE_GAMERUNNER_

#include "Engine\Engine\FrameTime.h"
#include "Engine\Engine\GameEngineConfig.h"

class GameEngine;

class GameRunner
{
private:

protected:

	friend class GameEngine;

	// Base functions.
	virtual void Preload() = 0;
	virtual void Start() = 0;
	virtual void End() = 0;
	virtual void Tick(const FrameTime& time) = 0;

	// Config functions
	virtual const GameEngineConfig& Get_Engine_Config() = 0;

};

#endif

