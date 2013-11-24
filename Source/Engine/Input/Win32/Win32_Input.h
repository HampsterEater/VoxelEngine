// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_WIN32INPUT_
#define _ENGINE_WIN32INPUT_

#include "Engine\Input\Input.h"
#include "Engine\Engine\FrameTime.h"
#include "Engine\Input\Win32\Win32_KeyboardState.h"
#include "Engine\Input\Win32\Win32_MouseState.h"

class Win32_Input : public Input
{
private:
	Win32_MouseState*		m_mouse_state;
	Win32_KeyboardState*	m_keyboard_state;

public:	

	Win32_Input();

	// Base functions.	
	void Tick(const FrameTime& time);
	
	// Mouse state.
	MouseState* Get_Mouse_State();
	
	// Key state.
	KeyboardState* Get_Keyboard_State();
	
};

#endif

