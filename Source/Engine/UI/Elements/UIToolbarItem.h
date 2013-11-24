// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UITOOLBARITEM_
#define _ENGINE_UI_ELEMENTS_UITOOLBARITEM_

#include "Engine\UI\UIElement.h"
#include "Engine\UI\UIFrame.h"
#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontRenderer.h"

#include "Engine\Renderer\Atlases\AtlasRenderer.h"

class AtlasHandle;
class UIManager;
class AtlasFrame;
class UIToolbar;

struct UIToolbarItemState
{
	enum Type
	{
		Normal,
		Hover,
		Pressed
	};
};

class UIToolbarItem : public UIElement 
{
protected:	
	friend class UILayout;
	friend class UIToolbar;

	FontHandle*				m_font;
	FontRenderer			m_font_renderer;

	AtlasHandle*			m_atlas;
	AtlasRenderer			m_atlas_renderer;

	UIManager*				m_manager;

	UIFrame					m_background_hover_frame;
	UIFrame					m_background_pressed_frame;
	UIFrame					m_background_seperator_frame;
	UIFrame					m_background_frame;

	UIToolbarItemState::Type m_state;

	std::string				m_icon_name;
	AtlasFrame*				m_icon;
	Point					m_icon_position;

	bool					m_seperator;
	int						m_seperator_height;

	bool					m_selected;

public:
	UIToolbarItem();
	~UIToolbarItem();
	
	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

	void Set_Selected(bool value);
	bool Get_Selected();

};

#endif

