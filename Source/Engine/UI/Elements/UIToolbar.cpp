// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\Elements\UIToolbar.h"
#include "Engine\UI\Elements\UIToolbarItem.h"
#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\AtlasRenderer.h"
#include "Engine\UI\UIManager.h"
#include "Engine\Engine\GameEngine.h"

UIToolbar::UIToolbar()
	: m_item_padding(5, 5, 5, 5)
{
	m_accepts_mouse_input = true;
}

UIToolbar::~UIToolbar()
{
}
	
void UIToolbar::Refresh()
{
	m_manager			= GameEngine::Get()->Get_UIManager();
	m_atlas				= m_manager->Get_Atlas();
	m_screen_box		= Calculate_Screen_Box();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	
	// Grab all atlas frames.
	m_background_frame = UIFrame(m_atlas, "toolbar_background_#");

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIToolbar::After_Refresh()
{
	// Are we horizontal or vertical?
	bool vertical = (m_screen_box.Height > m_screen_box.Width);
	float item_size = 0.0f;

	if (vertical == true)
	{
		item_size = m_screen_box.Width - (m_item_padding.X + m_item_padding.Width);
	}
	else
	{
		item_size = m_screen_box.Height - (m_item_padding.Y + m_item_padding.Height);
	}

	// Offset children to create a line within the bar.
	float item_offset = 0.0f;
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		UIToolbarItem* item = dynamic_cast<UIToolbarItem*>(*iter);
		if (item != NULL)
		{
			Rect box = item->Get_Screen_Box();
			if (vertical == true)
			{
				box.X = m_screen_box.X + m_item_padding.X;			
				box.Y = m_screen_box.Y + m_item_padding.Y + item_offset;				
			}
			else
			{
				box.Y = m_screen_box.Y + m_item_padding.Y;			
				box.X = m_screen_box.X + m_item_padding.X + item_offset;
			}
			
			if (item->m_seperator == true)
			{
				if (vertical == true)
				{
					box.Height	= m_item_padding.Height;
					box.Width	= item_size;
				}
				else
				{
					box.Height	= item_size;
					box.Width	= m_item_padding.Height;	
				}
			}
			else
			{
				box.Height	= item_size;
				box.Width	= item_size;		
			}

			item->Set_Screen_Box(box);

			if (vertical == true)
				item_offset += box.Height + m_item_padding.Height;
			else
				item_offset += box.Width + m_item_padding.Width;
		}
	}

	// After-refresh children.
	UIElement::After_Refresh();
}

void UIToolbar::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update items.	
	UIElement::Tick(time, manager, scene);
}

void UIToolbar::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Draw background.
	m_background_frame.Draw_Frame(time, manager, m_atlas_renderer, m_screen_box);

	// Draw each toolbar item.
	UIElement::Draw(time, manager, scene);
}


