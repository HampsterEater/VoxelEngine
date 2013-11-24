// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\Elements\UIContextMenu.h"
#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\AtlasRenderer.h"
#include "Engine\UI\UIFrame.h"
#include "Engine\UI\UIManager.h"
#include "Engine\Engine\GameEngine.h"
#include "Engine\Input\Input.h"

UIContextMenu::UIContextMenu()
	: m_inner_padding(2, 2, 4, 4)
	, m_state(UIMenuItemState::Normal)
{
}

UIContextMenu::~UIContextMenu()
{
}
	
void UIContextMenu::Refresh()
{
	m_manager			= GameEngine::Get()->Get_UIManager();
	m_atlas				= m_manager->Get_Atlas();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	m_font				= m_manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font);

	// Box is simply the size of the string at origin 0,0. Are placement is done by our parent.
	Point size			= m_font_renderer.Calculate_String_Size(m_value.c_str(), 16.0f);
	m_screen_box		= Rect(0, 0, size.X, size.Y);
	
	// Grab all atlas frames.
	m_background_hover_frame = UIFrame(m_atlas, "menuitem_background_hover_#");
}

void UIContextMenu::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input*		input	 = Input::Get();
	MouseState* mouse	 = input->Get_Mouse_State();
	Point		position = mouse->Get_Position();

	if (m_screen_box.Intersects(position))
	{
		m_state = UIMenuItemState::Hover;
	}
	else
	{
		m_state = UIMenuItemState::Normal;
	}
}

void UIContextMenu::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Draw background.
	if (m_state == UIMenuItemState::Hover)
	{
		m_background_hover_frame.Draw_Frame(time, manager, m_atlas_renderer, Rect(m_screen_box.X - m_inner_padding.X, m_screen_box.Y - m_inner_padding.Y, m_screen_box.Width + m_inner_padding.Width, m_screen_box.Height + m_inner_padding.Height));
	}

	// Draw text.
	m_font_renderer.Draw_String(time, m_value.c_str(), m_screen_box, TextAlignment::Center);
}

