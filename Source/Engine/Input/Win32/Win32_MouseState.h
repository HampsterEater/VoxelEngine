// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_WIN32_MOUSESTATE_
#define _ENGINE_INPUT_WIN32_MOUSESTATE_

#include "Engine/Input/MouseState.h"
#include "Engine\Engine\FrameTime.h"

class Win32_Input;

class Win32_MouseState : public MouseState
{
private:
	friend class Win32_Input;

	bool m_button_state[3];
	bool m_button_previous_state[3];

public:
	Win32_MouseState();

	void	Tick				(const FrameTime& time);

	bool	Is_Button_Down		(MouseButton::Type type);
	bool	Was_Button_Clicked	(MouseButton::Type type);

	void	Set_Position		(Point position);
	Point	Get_Position		();

};

#endif

