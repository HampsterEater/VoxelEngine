// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIMENUBAR_
#define _ENGINE_UI_ELEMENTS_UIMENUBAR_

#include "Engine\UI\UIElement.h"
#include "Engine\UI\UIFrame.h"

#include "Engine\Renderer\Atlases\AtlasRenderer.h"

class AtlasHandle;
class UIManager;
class AtlasFrame;

class UIMenuBar : public UIElement 
{
protected:	
	friend class UILayout;

	AtlasHandle*	m_atlas;
	AtlasRenderer	m_atlas_renderer;

	UIManager*		m_manager;
	UIFrame			m_background_frame;

	Rect			m_item_padding;

public:
	UIMenuBar();
	~UIMenuBar();
	
	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Post_Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

