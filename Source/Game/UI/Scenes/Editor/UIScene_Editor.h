// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_EDITOR_
#define _GAME_UI_SCENES_UISCENE_EDITOR_

#include "Engine\UI\UIScene.h"

#include "Game\Scene\Editor\EditorSelectionPrimitive.h"

#include "Game\Scene\Voxels\Voxel.h"

#include "Generic\Types\IntVector3.h"

class UIToolbarItem;

#define EDITOR_SELECTION_PRIMITIVE_PADDING 0.01f

struct EditorMode
{
	enum Type
	{
		Pencil,
		Sphere,
		Cube,

		COUNT
	};
};

class UIScene_Editor : public UIScene
{
private:
	EditorMode::Type			m_mode;
	IntVector3					m_selection_position;
	IntVector3					m_selection_normal;

protected:

public:
	UIScene_Editor();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	bool Should_Display_Cursor();
	
	void	   Set_Selection_Position(IntVector3 vector, IntVector3 normal);
	IntVector3 Set_Selection_Position();

	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);
	
	void Refresh(UIManager* manager);
	
	void Recieve_Event(UIEvent e);

	void Refresh_State();

	void Action_ChangeMode(EditorMode::Type type);	
	void Action_Add();
	void Action_Subtract();
	void Action_Cut();
	void Action_Copy();
	void Action_Paste();

	void Update_Tool_Pencil();
	void Update_Tool_Cube();
	void Update_Tool_Sphere();

};

#endif

