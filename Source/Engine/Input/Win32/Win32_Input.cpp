// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Input\Win32\Win32_Input.h"

Win32_Input::Win32_Input()
{
	m_mouse_state	 = new Win32_MouseState();
	m_keyboard_state = new Win32_KeyboardState();
}

void Win32_Input::Tick(const FrameTime& time)
{
	m_mouse_state->Tick(time);
	m_keyboard_state->Tick(time);
}
	
MouseState* Win32_Input::Get_Mouse_State()
{
	return m_mouse_state;
}
	
KeyboardState* Win32_Input::Get_Keyboard_State()
{
	return m_keyboard_state;
}
	
