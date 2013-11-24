// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\UIScene.h"
#include "Engine\UI\UIManager.h"
#include "Engine\UI\UIElement.h"
#include "Engine\UI\Layouts\UILayout.h"
#include "Engine\UI\Layouts\UILayoutHandle.h"
#include "Engine\UI\Layouts\UILayoutFactory.h"
#include "Engine\Engine\GameEngine.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"

UIScene::UIScene()
	: m_layout(NULL)
	, m_layout_last_version(-1)
{
}

UIScene::~UIScene()
{
	Clear_Elements();
}

void UIScene::Clear_Elements()
{
	for (LinkedList<UIElement*>::Iterator iter = m_elements.Begin(); iter != m_elements.End(); iter++)
	{
		delete *iter;
	}
	m_elements.Clear();
}

void UIScene::Enter(UIManager* manager)
{
}

void UIScene::Exit(UIManager* manager)
{
}

bool UIScene::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene::Should_Display_Cursor()
{
	return false;
}

UIElement* UIScene::Find_Element_By_Position(UIManager* manager, Point position, int scene_index, bool only_mouse_hot)
{
	// Check all elements.
	for (LinkedList<UIElement*>::Iterator iter = m_elements.Begin(); iter != m_elements.End(); iter++)
	{
		UIElement* element = *iter;
		UIElement* hit = element->Find_Element_By_Position(manager, position, only_mouse_hot);
		if (hit != NULL)
		{
			return hit;
		}
	}

	// Check lower scenes?
	if (Should_Render_Lower_Scenes() && scene_index > 0)
	{
		UIScene* scene = manager->Get_Scene(scene_index - 1);
		return scene->Find_Element_By_Position(manager, position, scene_index - 1, only_mouse_hot);
	}

	return NULL;
}

void UIScene::Set_Element_Value(const char* name, std::string& value)
{
	Set_Element_Value(name, value.c_str());
}

void UIScene::Set_Element_Value(const char* name, const char* value)
{
	int hash = StringHelper::Hash(name);
	for (LinkedList<UIElement*>::Iterator iter = m_elements.Begin(); iter != m_elements.End(); iter++)
	{
		UIElement* element = *iter;
		if (element->Get_Name_Hash() == hash)
		{
			element->Set_Value(value);
			return;
		}
	}
}

void UIScene::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	Pre_Draw(time, manager, scene_index);
	Post_Draw(time, manager, scene_index);
}

void UIScene::Pre_Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Render lower scenes?
	if (Should_Render_Lower_Scenes() && scene_index > 0)
	{
		UIScene* scene = manager->Get_Scene(scene_index - 1);
		scene->Draw(time, manager, scene_index - 1);
	}

	// Render all elements.
	for (LinkedList<UIElement*>::Iterator iter = m_elements.Begin(); iter != m_elements.End(); iter++)
	{
		(*iter)->Draw(time, manager, this);
	}
}

void UIScene::Post_Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	for (LinkedList<UIElement*>::Iterator iter = m_elements.Begin(); iter != m_elements.End(); iter++)
	{
		(*iter)->Post_Draw(time, manager, this);
	}
}


void UIScene::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Has the scene reloaded?
	if (m_layout->Get_Version() != m_layout_last_version)
	{
		Refresh(manager);
	}

	// Tick all elements.
	for (LinkedList<UIElement*>::Iterator iter = m_elements.Begin(); iter != m_elements.End(); iter++)
	{
		(*iter)->Tick(time, manager, this);
	}
}

void UIScene::Refresh(UIManager* manager)
{
	Instantiate_Layout();
}

void UIScene::Instantiate_Layout()
{
	Clear_Elements();

	// Instantiate elements and add to scene.
	std::vector<UIElement*> elements = m_layout->Get()->Instantiate();
	for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
	{
		UIElement* element = *iter;
		element->Refresh();

		m_elements.Add(element);
	}
	for (std::vector<UIElement*>::iterator iter = elements.begin(); iter != elements.end(); iter++)
	{
		UIElement* element = *iter;
		element->After_Refresh();
	}

	m_layout_last_version = m_layout->Get_Version();
}

void UIScene::Set_Layout(UILayoutHandle* layout)
{
	if (m_layout != layout)
	{
		m_layout = layout;
		Refresh(GameEngine::Get()->Get_UIManager());
	}
	else
	{
		m_layout = layout;
	}
}

void UIScene::Set_Layout(const char* path)
{
	Set_Layout(UILayoutFactory::Load(path));
}

void UIScene::Dispatch_Event(UIEvent e)
{
	DBG_LOG("UI Event '%s' was fired by element '%s'.", e.Name, e.Source->Get_Name().c_str());
	Recieve_Event(e);
}

void UIScene::Recieve_Event(UIEvent e)
{
}
