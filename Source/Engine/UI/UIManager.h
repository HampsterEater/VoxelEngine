// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIMANAGER_
#define _ENGINE_UI_UIMANAGER_

#include "Engine\Engine\FrameTime.h"

#include "Generic\Types\Point.h"

class UIScene;
class UITransition;
class UIElement;

class FontHandle;
class FontRenderer;
class AtlasHandle;
class AtlasRenderer;
class AtlasFrame;
class Camera;

#include <vector>

#define UI_MAX_SCENES 8

struct UICursorType
{
	enum Type
	{
		Pointer = 0,
		SplitH	= 1,
		SplitV	= 2,

		COUNT,
	};
};

class UIManager
{
protected:
	UIScene*				m_next_scene;
	int						m_next_scene_index;

	std::vector<UIScene*>	m_to_delete;

	UIScene*				m_scenes[UI_MAX_SCENES];
	int						m_scene_index;

	UITransition*			m_transition;

	float					m_scene_change_time;
	
	AtlasHandle*			m_ui_atlas;
	FontHandle*				m_ui_font;
	AtlasRenderer*			m_atlas_renderer;

	AtlasFrame*				m_cursor_frames[UICursorType::COUNT];
	UICursorType::Type		m_cursor_type;

	int						m_last_atlas_version;

	Camera*					m_camera;

public:
	UIManager();
	~UIManager();
	
	UIScene*	 Get_Scene(int offset = 0);
	AtlasHandle* Get_Atlas();
	FontHandle*	 Get_Font();

	Camera*		 Get_Camera();
	void		 Set_Camera(Camera* camera);

	void Set_Cursor(UICursorType::Type cursor);

	void Refresh();

	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time);

	void Pop (UITransition* transition = NULL, const char* until_scene_name = "");
	void Push(UIScene* scene, UITransition* transition = NULL);

	UIElement* Find_Element_By_Position(Point position, bool only_mouse_hot = false);

};

#endif

