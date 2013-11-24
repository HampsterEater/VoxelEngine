// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UICONTEXTMENU_
#define _ENGINE_UI_ELEMENTS_UICONTEXTMENU_

#include "Engine\UI\UIElement.h"

#include "Engine\Renderer\Atlases\AtlasRenderer.h"

class AtlasHandle;
class UIManager;
class AtlasFrame;

class UIContextMenu : public UIElement 
{
protected:	
	friend class UILayout;

	AtlasHandle*	m_atlas;
	AtlasRenderer	m_atlas_renderer;

	UIManager*		m_manager;
	AtlasFrame*		m_background_frame;

	Rect			m_item_padding;

public:
	UIContextMenu();
	~UIContextMenu();
	
	void Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

