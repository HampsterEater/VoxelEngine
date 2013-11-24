// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\Elements\UILabel.h"
#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\AtlasRenderer.h"
#include "Engine\UI\UIManager.h"
#include "Engine\Engine\GameEngine.h"

#include "Engine\Renderer\Text\FontRenderer.h"
#include "Engine\Localise\Locale.h"

UILabel::UILabel()
	: m_halign(TextAlignment::Left)
	, m_valign(TextAlignment::Top)
	, m_color(Color::Black)
{
}

UILabel::~UILabel()
{
}
	
void UILabel::Refresh()
{
	m_manager			= GameEngine::Get()->Get_UIManager();
	m_atlas				= m_manager->Get_Atlas();
	m_screen_box		= Calculate_Screen_Box();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	m_font				= m_manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font);
	
	// Grab all atlas frames.

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UILabel::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update items.	
	UIElement::Tick(time, manager, scene);
}

void UILabel::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Draw the text.
	m_font_renderer.Draw_String(time, S(m_value), Rect(m_screen_box.X + m_padding.X, m_screen_box.Y + m_padding.Y, m_screen_box.Width - m_padding.Width, m_screen_box.Height - m_padding.Height), m_color, m_halign, m_valign);

	// Draw each menu item.
	UIElement::Draw(time, manager, scene);
}

