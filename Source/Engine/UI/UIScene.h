// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UISCENE_
#define _ENGINE_UI_UISCENE_

#include "Engine\Engine\FrameTime.h"

#include "Generic\Types\LinkedList.h"

class UIManager;
class UIElement;

class UIScene
{
protected:
	LinkedList<UIElement*> m_elements;

protected:
	void Set_Layout(const char* layout_file_path);

public:
	UIScene();
	virtual ~UIScene();
	
	virtual const char* Get_Name() = 0;
	virtual bool Should_Render_Lower_Scenes() = 0;

	virtual void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	virtual void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

