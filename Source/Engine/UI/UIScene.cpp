// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\UIScene.h"
#include "Engine\UI\UIManager.h"
#include "Engine\UI\UIElement.h"

UIScene::UIScene()
{
}

void UIScene::Draw(const FrameTime& time, UIManager* manager, int scene_index)
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

void UIScene::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Render all elements.
	for (LinkedList<UIElement*>::Iterator iter = m_elements.Begin(); iter != m_elements.End(); iter++)
	{
		(*iter)->Tick(time, manager, this);
	}
}

void UIScene::Set_Layout(const char* layout_file_path)
{

}
