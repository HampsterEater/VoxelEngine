// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UISCENE_
#define _ENGINE_UI_UISCENE_

#include "Engine\Engine\FrameTime.h"

#include "Generic\Types\LinkedList.h"
#include "Generic\Types\Point.h"

#include "Engine\UI\UIEvent.h"

class UIManager;
class UIElement;
class UILayout;
class UILayoutHandle;

class UIScene
{
private:
	LinkedList<UIElement*>	m_elements;
	UILayoutHandle*			m_layout;
	int						m_layout_last_version;
	int						m_layout_last_atlas_version;

	void Clear_Elements();
	void Instantiate_Layout();
	
protected:
	void Set_Layout(UILayoutHandle* layout);
	void Set_Layout(const char* path);
	
	template<typename T>
	T Find_Element(const char* name)
	{
		int hash = StringHelper::Hash(name);
		for (LinkedList<UIElement*>::Iterator iter = m_elements.Begin(); iter != m_elements.End(); iter++)
		{
			UIElement* element = (*iter)->Find_Element<T>(hash);
			if (element != NULL)
			{
				return static_cast<T>(element);
			}
		}	
		return NULL;
	}
	
	void Set_Element_Value(const char* name, std::string& value);
	void Set_Element_Value(const char* name, const char* value);

public:
	UIScene();
	virtual ~UIScene();
	
	virtual const char* Get_Name() = 0;
	virtual bool Should_Render_Lower_Scenes();
	virtual bool Should_Display_Cursor();

	virtual void Enter(UIManager* manager);
	virtual void Exit(UIManager* manager);

	virtual void Refresh(UIManager* manager);

	virtual void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	virtual void Draw(const FrameTime& time, UIManager* manager, int scene_index);
	virtual void Pre_Draw(const FrameTime& time, UIManager* manager, int scene_index);
	virtual void Post_Draw(const FrameTime& time, UIManager* manager, int scene_index);
	
	void Dispatch_Event(UIEvent e);
	virtual void Recieve_Event(UIEvent e);
	
	UIElement* Find_Element_By_Position(UIManager* manager, Point position, int scene_index, bool only_mouse_hot);

};

#endif

