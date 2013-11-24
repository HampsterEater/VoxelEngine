// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\Elements\UIPanel.h"
#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\AtlasRenderer.h"
#include "Engine\UI\UIFrame.h"
#include "Engine\UI\UIManager.h"
#include "Engine\Engine\GameEngine.h"
#include "Engine\Input\Input.h"
#include "Engine\Platform\Platform.h"
#include "Engine\Localise\Locale.h"

UIPanel::UIPanel()
	: m_backcolor(Color(255, 255, 255, 255))
{
}

UIPanel::~UIPanel()
{
}

void UIPanel::Refresh()
{
	// Calculate screen-space box.
	m_screen_box = Calculate_Screen_Box();

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIPanel::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UIPanel::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIPanel::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Draw the background.
	m_primitive_renderer.Draw_Solid_Quad(time, m_screen_box, m_backcolor);

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
