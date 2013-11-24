// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_
#define _ENGINE_INPUT_

#include "Generic\Patterns\Singleton.h"
#include "Generic\Types\Point.h"
#include "Engine\Engine\FrameTime.h"

#include "Engine\Input\KeyboardState.h"
#include "Engine\Input\MouseState.h"

class Input : public Singleton<Input>
{
public:	
	static Input* Create();
	
	// Base functions.	
	virtual void Tick(const FrameTime& time) = 0;
	
	// Mouse state.
	virtual MouseState* Get_Mouse_State() = 0;

	// Key state.
	virtual KeyboardState* Get_Keyboard_State() = 0;

};

#endif

