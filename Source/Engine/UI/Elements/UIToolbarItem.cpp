// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\Elements\UIToolbarItem.h"
#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\AtlasRenderer.h"
#include "Engine\UI\UIFrame.h"
#include "Engine\UI\UIEvent.h"
#include "Engine\UI\UIScene.h"
#include "Engine\UI\UIManager.h"
#include "Engine\Engine\GameEngine.h"
#include "Engine\Input\Input.h"
#include "Engine\Platform\Platform.h"
#include "Engine\Localise\Locale.h"

UIToolbarItem::UIToolbarItem()
	: m_state(UIToolbarItemState::Normal)
	, m_icon_name("")
	, m_icon(NULL)
	, m_seperator_height(1)
	, m_seperator(false)
	, m_selected(false)
{
	m_accepts_mouse_input = true;
}

UIToolbarItem::~UIToolbarItem()
{
}

void UIToolbarItem::Refresh()
{
	m_manager			= GameEngine::Get()->Get_UIManager();
	m_atlas				= m_manager->Get_Atlas();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	m_font				= m_manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font, false, false);
	
	// Grab all atlas frames.
	m_background_hover_frame	 = UIFrame(m_atlas, "toolbaritem_background_hover_#");
	m_background_pressed_frame	 = UIFrame(m_atlas, "toolbaritem_background_pressed_#");
	m_background_seperator_frame = UIFrame(m_atlas, "toolbaritem_background_seperator_#");
	m_background_frame			 = UIFrame(m_atlas, "toolbaritem_background_#");
	m_icon					     = m_icon_name == "" ? NULL : m_atlas->Get()->Get_Frame(m_icon_name.c_str());
	
	if (m_icon == NULL)
	{
		m_seperator  = true;
		m_screen_box = Rect(0, 0, 1, m_seperator_height);
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIToolbarItem::After_Refresh()
{		
	// Calculate position of icon.
	if (m_icon != NULL)
	{
		m_icon_position = Point(m_screen_box.X + (m_screen_box.Width / 2) - (m_icon->Rectangle.Width / 2), 
							    m_screen_box.Y + (m_screen_box.Height / 2) - (m_icon->Rectangle.Height / 2));
	}

	// After-refresh children.
	UIElement::After_Refresh();
}

void UIToolbarItem::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input*		input	 = Input::Get();
	MouseState* mouse	 = input->Get_Mouse_State();
	Point		position = mouse->Get_Position();

	// Menu item selected?
	if (m_seperator == false && m_enabled == true)
	{
		if (m_screen_box.Intersects(position))
		{
			if (mouse->Was_Button_Clicked(MouseButton::Left))
			{
				scene->Dispatch_Event(UIEvent(UIEventType::ToolbarItem_Click, this));
			}
			else if (mouse->Is_Button_Down(MouseButton::Left))
			{
				m_state = UIToolbarItemState::Pressed;
			}
			else
			{
				m_state = UIToolbarItemState::Hover;
			}
		}
		else
		{
			m_state = UIToolbarItemState::Normal;
		}
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIToolbarItem::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	if (m_seperator == true)
	{
		//m_background_seperator_frame.Draw_Frame(time, manager, m_atlas_renderer, m_screen_box);
	}
	else
	{
		// Draw background.
		if (m_enabled == false)
		{
			m_background_frame.Draw_Frame(time, manager, m_atlas_renderer, m_screen_box);
		}
		else if (m_state == UIToolbarItemState::Pressed || m_selected == true)
		{
			m_background_pressed_frame.Draw_Frame(time, manager, m_atlas_renderer, m_screen_box);
		}
		else if (m_state == UIToolbarItemState::Hover)
		{
			m_background_hover_frame.Draw_Frame(time, manager, m_atlas_renderer, m_screen_box);
		}
		else
		{
			m_background_frame.Draw_Frame(time, manager, m_atlas_renderer, m_screen_box);
		}

		// Draw the icon.
		if (m_icon != NULL)
		{
			if (m_enabled == false)
			{
				m_atlas_renderer.Draw_Frame(time, m_icon, m_icon_position, Color(64, 64, 64, 64));
			}
			else
			{
				m_atlas_renderer.Draw_Frame(time, m_icon, m_icon_position, Color(255, 255, 255, 255));
			}
		}
	}
}

void UIToolbarItem::Set_Selected(bool value)
{
	m_selected = value;
}

bool UIToolbarItem::Get_Selected()
{
	return m_selected;
}

