// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIMANAGER_
#define _ENGINE_UI_UIMANAGER_

#include "Engine\Engine\FrameTime.h"

class UIScene;
class UITransition;

class FontHandle;
class FontRenderer;

#define UI_MAX_SCENES 8

class UIManager
{
protected:
	UIScene* m_previous_scene;
	UIScene* m_scenes[UI_MAX_SCENES];
	int m_scene_index;

	UITransition* m_transition;

	float m_scene_change_time;
	
	FontHandle*		m_font;
	FontRenderer*	m_font_renderer;

public:
	UIManager();
	~UIManager();
	
	UIScene* Get_Scene(int offset = 0);

	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time);

	void Pop (UITransition* transition = NULL);
	void Push(UIScene* scene, UITransition* transition = NULL);

};

#endif

