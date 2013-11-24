// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\UIElement.h"

#include "Engine\Display\Display.h"

#include "Engine\Engine\FrameTime.h"

#include "Generic\Helper\StringHelper.h"

UIElement::UIElement()
	: m_name_hash(0)
	, m_parent(NULL)
	, m_accepts_mouse_input(false)
	, m_enabled(true)
{
}

UIElement::~UIElement()
{
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		delete *iter;
	}
	m_children.clear();

	m_parent = NULL;
}

void UIElement::Set_Enabled(bool value)
{
	m_enabled = value;
}

bool UIElement::Get_Enabled()
{
	return m_enabled;
}

std::string UIElement::Get_Value()
{
	return m_value;
}

void UIElement::Set_Value(std::string value)
{
	m_value = value;
}

int UIElement::Get_Name_Hash()
{
	if (m_name_hash == 0)
	{
		m_name_hash = StringHelper::Hash(m_name.c_str());
	}
	return m_name_hash;
}

std::string UIElement::Get_Name()
{
	return m_name;
}

Rect UIElement::Get_Box()
{
	return m_box;
}

void UIElement::Set_Parent(UIElement* element)
{
	m_parent = element;
}

UIElement* UIElement::Get_Parent()
{
	return m_parent;
}

void UIElement::Refresh()
{
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		(*iter)->Refresh();
	}
}

void UIElement::After_Refresh()
{
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		(*iter)->After_Refresh();
	}
}

Rect UIElement::Get_Screen_Box()
{
	return m_screen_box;
}

void UIElement::Set_Screen_Box(Rect value)
{
	m_screen_box = value;
}

Rect UIElement::Calculate_Screen_Box()
{
	if (m_parent != NULL)
	{
		Rect parent_rect = m_parent->Get_Screen_Box();

		return Rect
		(
			parent_rect.X + (m_box.X <= 1.0f ? m_box.X * parent_rect.Width : m_box.X),
			parent_rect.Y + (m_box.Y <= 1.0f ? m_box.Y * parent_rect.Height : m_box.Y),
			m_box.Width <= 1.0f ? m_box.Width * parent_rect.Width : m_box.Width,
			m_box.Height <= 1.0f ? m_box.Height * parent_rect.Height : m_box.Height
		);
	}
	else
	{
		float screen_width  = Display::Get()->Get_Width();
		float screen_height = Display::Get()->Get_Height();

		return Rect
		(
			m_box.X <= 1.0f ? m_box.X * screen_width : m_box.X,
			m_box.Y <= 1.0f ? m_box.Y * screen_height : m_box.Y,
			m_box.Width <= 1.0f ? m_box.Width * screen_width : m_box.Width,
			m_box.Height <= 1.0f ? m_box.Height * screen_height : m_box.Height
		);
	}
}

void UIElement::Add_Child(UIElement* element)
{
	m_children.push_back(element);
	element->Set_Parent(this);
}

void UIElement::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{	
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		(*iter)->Tick(time, manager, scene);
	}
}

void UIElement::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		(*iter)->Draw(time, manager, scene);
	}
}

void UIElement::Post_Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		(*iter)->Post_Draw(time, manager, scene);
	}
}

UIElement* UIElement::Find_Element_By_Position(UIManager* manager, Point position, bool only_mouse_hot)
{
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UIElement* hit = (*iter)->Find_Element_By_Position(manager, position, only_mouse_hot);
		if (hit != NULL)
		{
			return hit;
		}
	}

	if (m_screen_box.Intersects(position) && (m_accepts_mouse_input == true || only_mouse_hot == false))
	{
		return this;
	}

	return NULL;
}
