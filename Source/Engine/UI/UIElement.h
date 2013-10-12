// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIELEMENT_
#define _ENGINE_UI_UIELEMENT_

#include "Engine\Engine\FrameTime.h"

class UIManager;
class UIScene;

class UIElement
{
protected:

public:
	UIElement();
	virtual ~UIElement();
	
	virtual const char* Get_Name() = 0;

	virtual void Tick(const FrameTime& time, UIManager* manager, UIScene* scene) = 0;
	virtual void Draw(const FrameTime& time, UIManager* manager, UIScene* scene) = 0;

};

#endif

