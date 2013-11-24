// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIELEMENT_
#define _ENGINE_UI_UIELEMENT_

#include "Engine\Engine\FrameTime.h"
#include "Generic\Types\Rectangle.h"

#include <vector>

class UIManager;
class UIScene;
class UILayout;

class UIElement
{
protected:
	friend class UILayout;
	
	std::string				m_value;
	int						m_name_hash;
	std::string				m_name;
	Rect					m_box;
	Rect					m_screen_box;
	std::vector<UIElement*> m_children;

	UIElement*				m_parent;

	bool					m_accepts_mouse_input;	

	bool					m_enabled;

public:
	UIElement();
	virtual ~UIElement();
		
	template<typename T>
	T Find_Element(const char* name)
	{
		int hash = StringHelper::Hash(name);
		return Find_Element<T>(hash)
	}
	
	template<typename T>
	T Find_Element(int hash)
	{
		if (m_name_hash == hash)
		{
			return static_cast<T>(this);
		}

		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			UIElement* element = (*iter)->Find_Element<T>(hash);
			if (element != NULL)
			{
				return static_cast<T>(element);
			}
		}	

		return NULL;
	}

	virtual void Add_Child(UIElement* element);

	virtual void Refresh();
	virtual void After_Refresh();

	virtual int			Get_Name_Hash();
	virtual std::string Get_Name();
	virtual std::string Get_Value();
	virtual void		Set_Value(std::string value);
	virtual Rect        Get_Box();
	virtual Rect        Get_Screen_Box();
	virtual void        Set_Screen_Box(Rect value);
	virtual Rect		Calculate_Screen_Box();
	virtual void		Set_Parent(UIElement* element);
	virtual UIElement*	Get_Parent();

	void Set_Enabled(bool value);
	bool Get_Enabled();

	virtual void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	virtual void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);
	virtual void Post_Draw(const FrameTime& time, UIManager* manager, UIScene* scene);
	
	virtual UIElement* Find_Element_By_Position(UIManager* manager, Point position, bool only_mouse_hot);

};

#endif

