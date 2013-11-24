// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIMENUITEM_
#define _ENGINE_UI_ELEMENTS_UIMENUITEM_

#include "Engine\UI\UIElement.h"
#include "Engine\UI\UIFrame.h"
#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontRenderer.h"

#include "Engine\Renderer\Atlases\AtlasRenderer.h"

class AtlasHandle;
class UIManager;
class AtlasFrame;

struct UIMenuItemState
{
	enum Type
	{
		Normal,
		Hover,
		Pressed
	};
};

class UIMenuItem : public UIElement 
{
protected:	
	friend class UILayout;

	FontHandle*				m_font;
	FontRenderer			m_font_renderer;

	AtlasHandle*			m_atlas;
	AtlasRenderer			m_atlas_renderer;

	UIManager*				m_manager;

	UIFrame					m_background_hover_frame;
	UIFrame					m_background_pressed_frame;
	UIFrame					m_context_background_frame;
	UIFrame					m_background_seperator_frame;
	AtlasFrame*				m_context_arrow_icon;

	Rect					m_inner_padding;
	Rect					m_outer_padding;

	UIMenuItemState::Type	m_state;
	bool					m_showing_context_menu;
	Rect					m_context_box;
	Rect					m_background_screen_box;

	std::string				m_icon_name;
	AtlasFrame*				m_icon;
	Point					m_icon_position;

	bool					m_is_in_context;

	bool					m_seperator;
	int						m_seperator_height;

	Rect					m_context_outer_padding;
	int						m_context_icon_offset;
	Point					m_context_icon_position;

	float					m_last_mouse_movement;
	Point					m_last_mouse_position;

	bool					m_disable_context_autoshow;
	float					m_open_context_hover_delay;

public:
	UIMenuItem();
	~UIMenuItem();
	
	void Refresh();
	void After_Refresh();

	void Set_Is_In_Context(bool is_context);

	void Show_Context_Menu();
	bool Point_Intersects_Context_Menus(Point point);
	void Collapse_Context_Menus();
	void Collapse_Context_Menus_To_Top();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);
	
	UIElement* Find_Element_By_Position(UIManager* manager, Point position, bool only_mouse_hot);

};

#endif

