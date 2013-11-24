// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UITRANSITION_
#define _ENGINE_UI_UITRANSITION_

#include "Engine\Engine\FrameTime.h"

class UIScene;
class UIManager;

class UITransition
{
protected:

public:
	UITransition();
	virtual ~UITransition();
	
	virtual const char* Get_Name() = 0;

	virtual bool Is_Complete() = 0;

	virtual void Tick(const FrameTime& time, UIManager* manager, UIScene* prev, UIScene* next, int scene_index, int next_scene_index) = 0;
	virtual void Draw(const FrameTime& time, UIManager* manager, UIScene* prev, UIScene* next, int scene_index, int next_scene_index) = 0;

};

#endif

