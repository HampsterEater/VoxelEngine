// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\Elements\UIMenuItem.h"
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

UIMenuItem::UIMenuItem()
	: m_inner_padding(2, 4, 4, 10)
	, m_outer_padding(5, 6, 10, 12)
	, m_state(UIMenuItemState::Normal)
	, m_showing_context_menu(false)
	, m_is_in_context(false)
	, m_icon_name("")
	, m_icon(NULL)
	, m_seperator_height(1)
	, m_seperator(false)
	, m_context_outer_padding(25, 4, 75, 8)
	, m_context_icon_offset(2)
	, m_last_mouse_movement(0)
	, m_last_mouse_position(0, 0)
	, m_open_context_hover_delay(500.0f)
	, m_disable_context_autoshow(false)
{
	m_accepts_mouse_input = true;
}

UIMenuItem::~UIMenuItem()
{
}

void UIMenuItem::Set_Is_In_Context(bool is_context)
{
	m_is_in_context = is_context;
}
	
void UIMenuItem::Refresh()
{
	m_manager			= GameEngine::Get()->Get_UIManager();
	m_atlas				= m_manager->Get_Atlas();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	m_font				= m_manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font, false, false);
	
	// Grab all atlas frames.
	m_background_hover_frame	 = UIFrame(m_atlas, "menuitem_background_hover_#");
	m_background_pressed_frame	 = UIFrame(m_atlas, "menuitem_background_pressed_#");
	m_context_background_frame	 = UIFrame(m_atlas, "menuitem_context_background_#");
	m_background_seperator_frame = UIFrame(m_atlas, "menuitem_background_seperator_#");
	m_icon					     = m_icon_name == "" ? NULL : m_atlas->Get()->Get_Frame(m_icon_name.c_str());
	m_context_arrow_icon		 = m_atlas->Get()->Get_Frame("menuitem_submenu_arrow_icon");
	
	// Box is simply the size of the string at origin 0,0. Are placement is done by our parent.
	Point size = m_font_renderer.Calculate_String_Size(m_value.c_str(), 16.0f);
	size.X = ceilf(size.X);
	size.Y = ceilf(size.Y);

	if (m_is_in_context == true)
	{
		size.X += m_context_outer_padding.Width;
	}

	if (m_icon != NULL)
	{
		m_screen_box = Rect(0, 0, m_icon->Rectangle.Width + m_inner_padding.Width + size.X, size.Y);		
	}
	else
	{
		m_screen_box = Rect(0, 0, size.X, size.Y);
	}

	if (m_value == "")
	{
		m_seperator  = true;
		m_screen_box = Rect(0, 0, 1, m_seperator_height);
	}

	// Set all children as context-menu items.
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UIMenuItem* item = dynamic_cast<UIMenuItem*>(*iter);
		if (item != NULL)
		{
			item->Set_Is_In_Context(true);
		}
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIMenuItem::After_Refresh()
{
	// Calculate background screen box.
	if (m_is_in_context == true)
	{
		m_background_screen_box = Rect(m_screen_box.X - m_inner_padding.X, m_screen_box.Y - m_inner_padding.Y, m_screen_box.Width + m_inner_padding.Width, m_screen_box.Height + m_inner_padding.Height);	
	}
	else
	{
		m_background_screen_box = Rect(m_screen_box.X - m_inner_padding.X, m_screen_box.Y - m_inner_padding.Y, m_screen_box.Width + m_inner_padding.Width, m_screen_box.Height + m_inner_padding.Height - 3);
	}
	
	// Offset screen box for icon.
	if (m_is_in_context == true)
	{
		m_screen_box.X += m_context_outer_padding.X;
		m_screen_box.Width -= m_context_outer_padding.X;
	}

	// Offset children to create a context-menu.
	float item_offset = 0.0f;
	float item_height = m_screen_box.Height;

	// If we are already a context menu we show to the right of this item.
	if (m_is_in_context == true)
	{
		m_context_box = Rect(m_background_screen_box.X + m_background_screen_box.Width - 1, m_background_screen_box.Y, 0, 0);
	}
	else
	{
		m_context_box = Rect(m_background_screen_box.X, m_background_screen_box.Y + m_background_screen_box.Height - 1, 0, 0);
	}

	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UIMenuItem* item = dynamic_cast<UIMenuItem*>(*iter);
		if (item != NULL)
		{
			Rect box	= item->Get_Screen_Box();
			box.X		= m_context_box.X + m_outer_padding.X;
			box.Y		= m_context_box.Y + m_context_box.Height + m_outer_padding.Y + 1;

			if (item->m_seperator == true)
			{
				box.Height = item->m_seperator_height;
			}
			else
			{
				box.Height = item_height;
			}

			item->Set_Screen_Box(box);
			item->Set_Is_In_Context(true);

			if (box.Width - m_outer_padding.Width > m_context_box.Width)
			{
				m_context_box.Width = box.Width - m_outer_padding.Width;
			}

			if (item->m_seperator == true)
			{
				m_context_box.Height += box.Height + (m_outer_padding.Height) - 3;
			}
			else
			{
				m_context_box.Height += box.Height + (m_outer_padding.Height);
			}
		}
	}

	m_context_box.Height += m_outer_padding.Y - 2;
	
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UIMenuItem* item = dynamic_cast<UIMenuItem*>(*iter);
		if (item != NULL)
		{
			Rect box	= item->Get_Screen_Box();
			box.Width	= m_context_box.Width - m_outer_padding.Width;
			item->Set_Screen_Box(box);
		}
	}
	
	// Calculate position of context icon.
	int icon_width  = m_context_arrow_icon->Rectangle.Width;
	int icon_height = m_context_arrow_icon->Rectangle.Height;
	m_context_icon_position = Point(m_screen_box.X + m_screen_box.Width - icon_width - m_context_icon_offset, m_screen_box.Y + (m_screen_box.Height / 2) - (icon_height / 2));

	// Calculate position of icon.
	if (m_icon != NULL)
	{
		m_icon_position = Point(m_background_screen_box.X + 2, m_background_screen_box.Y + (m_background_screen_box.Height / 2) - (m_icon->Rectangle.Height / 2));
	}

	// After-refresh children.
	UIElement::After_Refresh();
}

bool UIMenuItem::Point_Intersects_Context_Menus(Point point)
{
	if (m_showing_context_menu == true && m_context_box.Intersects(point))
	{
		return true;
	}
	else if (m_background_screen_box.Intersects(point))
	{
		return true;
	}
	else
	{
		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			UIMenuItem* item = dynamic_cast<UIMenuItem*>(*iter);
			if (item != NULL)
			{
				if (item->Point_Intersects_Context_Menus(point))
				{
					return true;
				}
			}
		}
		return false;
	}
}

void UIMenuItem::Collapse_Context_Menus()
{
	m_showing_context_menu = false;
	
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UIMenuItem* item = dynamic_cast<UIMenuItem*>(*iter);
		if (item != NULL)
		{
			item->Collapse_Context_Menus();
		}
	}
}

void UIMenuItem::Collapse_Context_Menus_To_Top()
{
	Collapse_Context_Menus();
	if (m_parent != NULL)
	{
		UIMenuItem* item = dynamic_cast<UIMenuItem*>(m_parent);
		if (item != NULL)
		{
			item->Collapse_Context_Menus_To_Top();
		}
	}
}

void UIMenuItem::Show_Context_Menu()
{
	m_showing_context_menu = true;
}

void UIMenuItem::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input*		input	 = Input::Get();
	MouseState* mouse	 = input->Get_Mouse_State();
	Point		position = mouse->Get_Position();

	// Menu item selected?
	if (m_seperator == false)
	{
		if (position != m_last_mouse_position)
		{
			m_last_mouse_movement = Platform::Get()->Get_Ticks();
			m_last_mouse_position = position;
			m_disable_context_autoshow = false;
		}

		if (m_background_screen_box.Intersects(position))
		{
			if (mouse->Was_Button_Clicked(MouseButton::Left))
			{
				if (m_children.size() > 0)
				{
					if (!m_showing_context_menu)
					{
						Show_Context_Menu();
					}
					else
					{
						m_showing_context_menu = false;
						m_disable_context_autoshow = true;
					}
				}
				else
				{
					scene->Dispatch_Event(UIEvent(UIEventType::MenuItem_Click, this));
					Collapse_Context_Menus_To_Top();
				}
			}
			else if (mouse->Is_Button_Down(MouseButton::Left))
			{
				m_state = UIMenuItemState::Pressed;
			}
			else
			{
				m_state = UIMenuItemState::Hover;

				// Open context menu if we hover over it for a few seconds.
				//float elapsed = Platform::Get()->Get_Ticks() - m_last_mouse_movement;
				//if (m_disable_context_autoshow == false && m_children.size() > 0 && elapsed > m_open_context_hover_delay)
				//{
				//	Show_Context_Menu();
				//}
			}
		}
		else
		{
			if (mouse->Was_Button_Clicked(MouseButton::Left) &&
				Point_Intersects_Context_Menus(position) == false)
			{
				Collapse_Context_Menus();
			}

			m_state = UIMenuItemState::Normal;
		}
	}

	// Update children.
	if (m_showing_context_menu == true)
	{
		UIElement::Tick(time, manager, scene);
	}
}

void UIMenuItem::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	if (m_seperator == true)
	{
		m_background_seperator_frame.Draw_Frame(time, manager, m_atlas_renderer, m_screen_box);
	}
	else
	{
		// Draw background.
		if (m_state == UIMenuItemState::Pressed || m_showing_context_menu == true)
		{
			m_background_pressed_frame.Draw_Frame(time, manager, m_atlas_renderer, m_background_screen_box);
		}
		else if (m_state == UIMenuItemState::Hover)
		{
			m_background_hover_frame.Draw_Frame(time, manager, m_atlas_renderer, m_background_screen_box);
		}

		// Draw text.
		if (m_is_in_context == true)
		{
			m_font_renderer.Draw_String(time, S(m_value), Rect(m_screen_box.X, m_screen_box.Y + 1, m_screen_box.Width, m_screen_box.Height), Color::Black, TextAlignment::Left, TextAlignment::Center);
		}
		else
		{
			m_font_renderer.Draw_String(time, S(m_value), Rect(m_screen_box.X, m_screen_box.Y - 1, m_screen_box.Width, m_screen_box.Height), Color::Black, TextAlignment::Center, TextAlignment::Center);
		}

		// Draw sub menu arrow icon.
		if (m_children.size() > 0 && m_is_in_context == true)
		{
			m_atlas_renderer.Draw_Frame(time, m_context_arrow_icon, m_context_icon_position);
		}

		// Draw the icon.
		if (m_icon != NULL)
		{
			m_atlas_renderer.Draw_Frame(time, m_icon, m_icon_position);
		}

		// Draw the sub-menu.
		if (m_showing_context_menu == true)
		{
			// Draw background.
			m_context_background_frame.Draw_Frame(time, manager, m_atlas_renderer, m_context_box);

			// Draw child menu items in sub-menu.
			UIElement::Draw(time, manager, scene);

			// Draw sub-menu vertical seperator.
			m_background_seperator_frame.Draw_Frame(time, manager, m_atlas_renderer, Rect(m_context_box.X + m_context_outer_padding.X, m_context_box.Y + m_context_outer_padding.Y, m_seperator_height, m_context_box.Height - m_context_outer_padding.Height));
		}
	}
}

UIElement* UIMenuItem::Find_Element_By_Position(UIManager* manager, Point position, bool only_mouse_hot)
{
	if (m_showing_context_menu == true)
	{
		if (m_context_box.Intersects(position))
		{
			return this;
		}
		return UIElement::Find_Element_By_Position(manager, position, only_mouse_hot);
	}
	else
	{
		return NULL;
	}
}