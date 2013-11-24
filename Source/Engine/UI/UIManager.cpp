// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\UIManager.h"
#include "Engine\UI\UIScene.h"
#include "Engine\UI\UITransition.h"

#include "Engine\UI\Layouts\UILayout.h"
#include "Engine\UI\Layouts\UILayoutFactory.h"
#include "Engine\UI\Layouts\UILayoutHandle.h"

#include "Engine\Platform\Platform.h"

#include "Generic\Math\Math.h"

#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontFactory.h"
#include "Engine\Renderer\Text\FontRenderer.h"

#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasFactory.h"

#include "Engine\Input\Input.h"

UIManager::UIManager()
	: m_scene_index(-1)
	, m_scene_change_time(0.0f)
	, m_transition(NULL)
	, m_next_scene(NULL)
	, m_next_scene_index(0)
	, m_last_atlas_version(-1)
	, m_cursor_type(UICursorType::Pointer)
	, m_camera(NULL)
{
	// Load the UI atlas.
	m_ui_atlas = AtlasFactory::Load("Data/Atlases/UI/ui.xml");
	DBG_ASSERT(m_ui_atlas != NULL);

	m_atlas_renderer = new AtlasRenderer(m_ui_atlas);

	// Load the UI font.
	m_ui_font = FontFactory::Load("Data/Fonts/ui.xml", FontFlags::None);
	DBG_ASSERT(m_ui_font != NULL);

	// Refresh resources.
	Refresh();
}

UIManager::~UIManager()
{
	SAFE_DELETE(m_atlas_renderer);

	while (m_scene_index-- >= 0)
	{
		SAFE_DELETE(m_scenes[m_scene_index + 1]);
	}

	SAFE_DELETE(m_next_scene);
	SAFE_DELETE(m_transition);
}

UIScene* UIManager::Get_Scene(int offset)
{
	return m_scenes[m_scene_index - (offset + 1)];
}

AtlasHandle* UIManager::Get_Atlas()
{
	return m_ui_atlas;
}

FontHandle* UIManager::Get_Font()
{
	return m_ui_font;
}

Camera*	UIManager::Get_Camera()
{
	return m_camera;
}

void UIManager::Set_Camera(Camera* camera)
{
	m_camera = camera;
}

void UIManager::Set_Cursor(UICursorType::Type cursor)
{
	m_cursor_type = cursor;
}

UIElement* UIManager::Find_Element_By_Position(Point position, bool only_mouse_hot)
{
	UIScene* curr_scene = m_scenes[m_scene_index];

	UIElement* element = curr_scene->Find_Element_By_Position(this, position, m_scene_index, only_mouse_hot);
	if (element != NULL)
	{
		return element;
	}

	return NULL;
}

void UIManager::Refresh()
{
	// Load cursors.
	m_cursor_frames[(int)UICursorType::Pointer] = m_ui_atlas->Get()->Get_Frame("cursor_pointer");
	m_cursor_frames[(int)UICursorType::SplitH]  = m_ui_atlas->Get()->Get_Frame("cursor_split_horizontal");
	m_cursor_frames[(int)UICursorType::SplitV]  = m_ui_atlas->Get()->Get_Frame("cursor_split_vertical");

	// Refresh scenes.
	for (int i = 0; i <= m_scene_index; i++)
	{
		m_scenes[i]->Refresh(this);
	}

	// Stores last version.
	m_last_atlas_version = m_ui_atlas->Get_Version();
}

void UIManager::Tick(const FrameTime& time)
{
	UIScene* curr_scene = m_scenes[m_scene_index];

	// Refresh states.	
	if (m_last_atlas_version != m_ui_atlas->Get_Version())
	{
		Refresh();
	}

	// Pending scene delete?
	for (std::vector<UIScene*>::iterator iter = m_to_delete.begin(); iter != m_to_delete.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}

	// Reset cursor to default.
	m_cursor_type = UICursorType::Pointer;

	if (m_transition != NULL)
	{
		m_transition->Tick(time, this, curr_scene, m_next_scene, m_scene_index, m_next_scene_index);
		if (m_transition->Is_Complete())
		{
			int offset = m_next_scene_index;
			for (int i = m_next_scene_index; i < m_scene_index; i++)
			{
				UIScene* scene = m_scenes[i];
				SAFE_DELETE(scene);
			}
			SAFE_DELETE(m_transition);
		}
	}
	else if (curr_scene != NULL)
	{
		curr_scene->Tick(time, this, m_scene_index);
	}
}

void UIManager::Draw(const FrameTime& time)
{
	Input* input = Input::Get();
	UIScene* curr_scene = m_scenes[m_scene_index];

	if (m_transition != NULL)
	{
		m_transition->Draw(time, this, curr_scene, m_next_scene, m_scene_index, m_next_scene_index);
	}
	else if (curr_scene != NULL)
	{
		curr_scene->Draw(time, this, m_scene_index);
	}

	// Draw mouse cursor.
	if (curr_scene != NULL && curr_scene->Should_Display_Cursor())
	{
		m_atlas_renderer->Draw_Frame(time, m_cursor_frames[m_cursor_type], input->Get_Mouse_State()->Get_Position().Round());
	}
}

void UIManager::Pop(UITransition* transition, const char* until_scene_name)
{
	DBG_ASSERT(m_scene_index >= 0);

	if (until_scene_name[0] == '\0')
	{	
		m_scenes[m_scene_index]->Exit(this);

		if (transition != NULL)
		{
			m_next_scene = m_scenes[m_scene_index - 1];
			m_next_scene_index = m_scene_index--;
		}
		else
		{
			m_to_delete.push_back(m_scenes[m_scene_index]);
			m_scenes[m_scene_index] = NULL;
			m_scene_index--;
		}
		
		if (m_scene_index >= 0)
		{
			m_scenes[m_scene_index]->Enter(this);
		}
	}
	else
	{
		int offset = 0;
		
		if (transition != NULL)
		{
			m_next_scene_index = m_scene_index;
			while (--m_next_scene_index >= 0)
			{
				if (stricmp(m_scenes[m_next_scene_index]->Get_Name(), until_scene_name) == 0)
				{
					m_next_scene = m_scenes[m_next_scene_index];
					m_scenes[m_next_scene_index]->Enter(this);
					break;
				}
				else
				{					
					m_scenes[m_next_scene_index]->Exit(this);
				}
			}
			DBG_ASSERT_STR(m_next_scene_index >= 0, "Failed to pop UI scenes until '%s'. Dosen't exist on scene stack.", until_scene_name);
		}
		else
		{
			while (m_scene_index >= 0)
			{
				if (stricmp(m_scenes[m_scene_index]->Get_Name(), until_scene_name) == 0)
				{
					m_scenes[m_scene_index]->Enter(this);
					break;
				}
			
				m_scenes[m_scene_index]->Exit(this);
		
				m_to_delete.push_back(m_scenes[m_scene_index]);
				m_scenes[m_scene_index] = NULL;

				m_scene_index--;
			}
			DBG_ASSERT_STR(m_scene_index >= 0, "Failed to pop UI scenes until '%s'. Dosen't exist on scene stack.", until_scene_name);
		}
	}
	m_transition = transition;
	m_scene_change_time = Platform::Get()->Get_Ticks();

	if (transition != NULL)
	{
		DBG_LOG("Changed to scene '%s' (index %i) with transition '%s'.", m_scenes[m_scene_index]->Get_Name(), m_scene_index, transition->Get_Name());
	}
	else
	{
		DBG_LOG("Changed to scene '%s' (index %i).", m_scenes[m_scene_index]->Get_Name(), m_scene_index);	
	}
}

void UIManager::Push(UIScene* scene, UITransition* transition)
{
	DBG_ASSERT(m_scene_index < UI_MAX_SCENES - 1);
	
	if (m_scene_index >= 0)
	{
		m_scenes[m_scene_index]->Exit(this);
	}

	if (transition != NULL)
	{
		m_next_scene = scene;
		m_next_scene_index = m_scene_index++;
	}
	else
	{
		m_scenes[++m_scene_index] = scene;	
	}
	m_transition = transition;
	m_scene_change_time = Platform::Get()->Get_Ticks();

	m_scenes[m_scene_index]->Enter(this);

	if (transition != NULL)
	{
		DBG_LOG("Changed to scene '%s' (index %i) with transition '%s'.", scene->Get_Name(), m_scene_index, transition->Get_Name());
	}
	else
	{
		DBG_LOG("Changed to scene '%s' (index %i).", scene->Get_Name(), m_scene_index);	
	}
}
