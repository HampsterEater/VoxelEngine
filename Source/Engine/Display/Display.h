// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_DISPLAY_
#define _ENGINE_DISPLAY_

#include "Generic\Patterns\Singleton.h"
#include "Generic\Types\Point.h"
#include "Engine\Engine\FrameTime.h"
#include "Engine\Display\Keys.h"

class Display : public Singleton<Display>
{
public:
	static Display* Create(const char* title, int width, int height, bool fullscreen);
	
	// Base functions.	
	virtual void Tick(const FrameTime& time) = 0;
	virtual void Swap_Buffers() = 0;

	// Properties
	virtual const char* Get_Title() = 0;
	virtual int Get_Width() = 0;
	virtual int Get_Height() = 0;
	virtual int Get_Fullscreen() = 0;

	// Modifiers
	virtual void Set_Title(const char* title) = 0;
	virtual void Resize(int width, int height, bool fullscreen) = 0;
	virtual void Toggle_Fullscreen() = 0;

	// Input
	// TODO: Move this into its own input platform-independent input class.
	virtual Point Get_Mouse() = 0;
	virtual void  Set_Mouse(Point pos) = 0;
	virtual bool  Is_Active() = 0;
	virtual bool  Is_Key_Down(Key::Type key) = 0;
	virtual bool  Is_Key_Pressed(Key::Type key) = 0;

};

#endif

