// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UILABEL_
#define _ENGINE_UI_ELEMENTS_UILABEL_

#include "Engine\UI\UIElement.h"
#include "Engine\UI\UIFrame.h"
#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontRenderer.h"

#include "Engine\Renderer\Atlases\AtlasRenderer.h"

#include "Generic\Types\Color.h"

class AtlasHandle;
class UIManager;
class AtlasFrame;

class UILabel : public UIElement 
{
protected:	
	friend class UILayout;

	AtlasHandle*			m_atlas;
	AtlasRenderer			m_atlas_renderer;
	
	FontHandle*				m_font;
	FontRenderer			m_font_renderer;

	UIManager*				m_manager;
	AtlasFrame*				m_background_frame;

	TextAlignment::Type		m_halign;
	TextAlignment::Type		m_valign;

	Rect					m_padding;

	Color					m_color;

public:
	UILabel();
	~UILabel();
	
	void Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

