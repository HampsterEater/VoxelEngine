// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_WIN32_KEYBOARDSTATE_
#define _ENGINE_INPUT_WIN32_KEYBOARDSTATE_

#include "Engine/Input/KeyboardState.h"
#include "Engine\Engine\FrameTime.h"

class Win32_Input;

class Win32_KeyboardState : public KeyboardState
{
private:
	friend class Win32_Input;
	
	bool m_button_state[256];
	bool m_button_previous_state[256];

public:
	Win32_KeyboardState();

	void Tick			(const FrameTime& time);
	
	bool Is_Key_Down	(KeyboardKey::Type type);
	bool Was_Key_Pressed(KeyboardKey::Type type);

};

#endif

