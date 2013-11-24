// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\Elements\UISplitContainerItem.h"
#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\AtlasRenderer.h"
#include "Engine\UI\UIFrame.h"
#include "Engine\UI\UIManager.h"
#include "Engine\Engine\GameEngine.h"
#include "Engine\Input\Input.h"
#include "Engine\Platform\Platform.h"
#include "Engine\Localise\Locale.h"

UISplitContainerItem::UISplitContainerItem()
	: m_size(0)
	, m_fixed(false)
{
}

UISplitContainerItem::~UISplitContainerItem()
{
}

void UISplitContainerItem::Refresh()
{
	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UISplitContainerItem::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UISplitContainerItem::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UISplitContainerItem::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Draw children.
	UIElement::Draw(time, manager, scene);
}
