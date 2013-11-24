// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_MOUSESTATE_
#define _ENGINE_INPUT_MOUSESTATE_

#include "Generic/Types/Point.h"

struct MouseButton
{
	enum Type
	{
		Left = 0,
		Right,
		Middle
	};
};

class MouseState
{
public:
	virtual bool	Is_Button_Down		(MouseButton::Type type) = 0;
	virtual bool	Was_Button_Clicked	(MouseButton::Type type) = 0;

	virtual void	Set_Position		(Point position) = 0;
	virtual Point	Get_Position		() = 0;

};

#endif

