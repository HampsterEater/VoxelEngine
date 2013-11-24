// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\UI\Scenes\Editor\UIScene_Editor.h"
#include "Engine\UI\Layouts\UILayoutFactory.h"

#include "Engine\Display\Display.h"

#include "Engine\Input\Input.h"

#include "Engine\UI\UIElement.h"
#include "Engine\UI\UIManager.h"
#include "Engine\UI\Elements\UIToolbarItem.h"

#include "Engine\Renderer\RenderPipeline.h"

#include "Engine\Engine\GameEngine.h"

#include "Game\Scene\Voxels\ChunkManager.h"

#include "Game\Scene\Editor\EditorGrid.h"

#include "Generic\Math\Math.h"

#include "Generic\Helper\StringHelper.h"

#include "Game\Runner\Game.h"

#include "Game\Version.h"
#include "Engine\Version.h"

UIScene_Editor::UIScene_Editor()
	: m_mode(EditorMode::Pencil)
	, m_selection_position(0, 0, 0)
	, m_selection_normal(0, 0, 0)
{
	Set_Layout("Data/Config/UI/Editor.xml");
}

const char* UIScene_Editor::Get_Name()
{
	return "UIScene_Editor";
}

bool UIScene_Editor::Should_Render_Lower_Scenes()
{
	return false;
}

bool UIScene_Editor::Should_Display_Cursor()
{
	Game* game = Game::Get();

	for (int i = 0; i < CameraID::COUNT; i++)
	{
		if (i != CameraID::Editor_Main &&
			i != CameraID::Editor_OrthoX &&
			i != CameraID::Editor_OrthoY &&
			i != CameraID::Editor_OrthoZ)
			continue;

		EditorCamera* editor_camera = static_cast<EditorCamera*>(game->Get_Camera((CameraID::Type)i));
		if (editor_camera->IsUnderMouseControl())
		{
			return false;
		}
	}

	return true;
}

void UIScene_Editor::Set_Selection_Position(IntVector3 vector, IntVector3 normal)
{
	m_selection_position = vector;
	m_selection_normal = normal;
}

IntVector3 UIScene_Editor::Set_Selection_Position()
{
	return m_selection_position;
}
	
void UIScene_Editor::Enter(UIManager* manager)
{
	// Copy game camera state to editor and begin using it.
	RenderPipeline*		pipeline	= RenderPipeline::Get();
	Game*				game		= Game::Get();
	const GameConfig&	config		= game->Get_Config();
	Camera* game_camera				= Game::Get()->Get_Camera(CameraID::Game);

	for (int i = 0; i < CameraID::COUNT; i++)
	{
		if (i != CameraID::Editor_Main &&
			i != CameraID::Editor_OrthoX &&
			i != CameraID::Editor_OrthoY &&
			i != CameraID::Editor_OrthoZ)
			continue;

		Camera* editor_camera = Game::Get()->Get_Camera((CameraID::Type)i);
		if (i == CameraID::Editor_Main)
		{
			editor_camera->Set_Position(game_camera->Get_Position());
			editor_camera->Set_Rotation(game_camera->Get_Rotation());
		}
		editor_camera->Set_Enabled(true);
	}

	// Enable editor geometry rendering.
	pipeline->Get_Pass_From_Name("editor-geometry")->Enabled = true;

	// Refresh state.
	Refresh_State();
}	

void UIScene_Editor::Exit(UIManager* manager)
{
	RenderPipeline* pipeline = RenderPipeline::Get();
	Camera* game_camera = Game::Get()->Get_Camera(CameraID::Game);
	Camera* editor_camera = Game::Get()->Get_Camera(CameraID::Editor_Main);

	for (int i = 0; i < CameraID::COUNT; i++)
	{
		if (i != CameraID::Editor_Main &&
			i != CameraID::Editor_OrthoX &&
			i != CameraID::Editor_OrthoY &&
			i != CameraID::Editor_OrthoZ)
			continue;
		
		EditorCamera* cam = static_cast<EditorCamera*>(Game::Get()->Get_Camera((CameraID::Type)i));
		cam->Set_Enabled(false);
		cam->Set_Editor_Scene(NULL);
	}

	// Disable editor geometry rendering.
	pipeline->Get_Pass_From_Name("editor-geometry")->Enabled = false;

	// Reset game camera to same position as editor camera.
	game_camera->Set_Position(editor_camera->Get_Position());
	game_camera->Set_Rotation(editor_camera->Get_Rotation());

	pipeline->Set_Active_Camera(game_camera);
}

void UIScene_Editor::Refresh(UIManager* manager)
{
	// Refresh base-scene stuff first (instantiate layout etc).
	UIScene::Refresh(manager);

	// Set version text.
#ifdef DEBUG_BUILD
	Set_Element_Value("versioninfo", StringHelper::Format("ED=%s  GD=%s", EngineAutoVersion::FULLVERSION_STRING, GameAutoVersion::FULLVERSION_STRING));
#else
	Set_Element_Value("versioninfo", StringHelper::Format("ER=%s  GR=%s", EngineAutoVersion::FULLVERSION_STRING, GameAutoVersion::FULLVERSION_STRING));
#endif
}

void UIScene_Editor::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	Display*		display			= Display::Get();
	Camera*			last_camera		= RenderPipeline::Get()->Get_Active_Camera();
	RenderPipeline* pipeline		= RenderPipeline::Get();
	Renderer*		renderer		= Renderer::Get();
	Game*			game			= Game::Get();
	ChunkManager*	chunk_manager	= game->Get_Chunk_Manager();
	Input*			input			= Input::Get();
	MouseState*		mouse_state		= input->Get_Mouse_State();
	const GameConfig&	config		= game->Get_Config();

	EditorSelectionPrimitive* mouse_primitive = game->Get_Editor_Mouse_Selection_Primitive();
	EditorSelectionPrimitive* main_primitive  = game->Get_Editor_Main_Selection_Primitive();

	// Back to game?
	if (Input::Get()->Get_Keyboard_State()->Was_Key_Pressed(KeyboardKey::F1))
	{
		manager->Pop();
	}

	// Update based on type.
	switch (m_mode)
	{
	case EditorMode::Pencil:	Update_Tool_Pencil();	break;
	case EditorMode::Cube:		Update_Tool_Cube();		break;
	case EditorMode::Sphere:	Update_Tool_Sphere();	break;
	}

	// Position mouse selection box.
	mouse_primitive->Set_Position(Vector3
	(
		(m_selection_position.X * config.chunk_config.voxel_size.X) - (EDITOR_SELECTION_PRIMITIVE_PADDING),
		(m_selection_position.Y * config.chunk_config.voxel_size.Y) - (EDITOR_SELECTION_PRIMITIVE_PADDING),
		(m_selection_position.Z * config.chunk_config.voxel_size.Z) - (EDITOR_SELECTION_PRIMITIVE_PADDING)	
	));
	mouse_primitive->Set_Size(Vector3(
		config.chunk_config.voxel_size.X + (EDITOR_SELECTION_PRIMITIVE_PADDING * 2),
		config.chunk_config.voxel_size.Y + (EDITOR_SELECTION_PRIMITIVE_PADDING * 2),
		config.chunk_config.voxel_size.Z + (EDITOR_SELECTION_PRIMITIVE_PADDING * 2)
	));

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Editor::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Get some junk we need.
	Display*		display		= Display::Get();
	Camera*			last_camera = RenderPipeline::Get()->Get_Active_Camera();
	RenderPipeline* pipeline	= RenderPipeline::Get();
	Renderer*		renderer	= Renderer::Get();
	Game*			game		= Game::Get();

	// Grab all the elements we care for.
	UIElement* camera_panel_tl = Find_Element<UIElement*>("camera_panel_tl");
	UIElement* camera_panel_tr = Find_Element<UIElement*>("camera_panel_tr");
	UIElement* camera_panel_bl = Find_Element<UIElement*>("camera_panel_bl");
	UIElement* camera_panel_br = Find_Element<UIElement*>("camera_panel_br");

	// Draw all pre scene elements.
	UIScene::Pre_Draw(time, manager, scene_index);

	// Map camera panels to camera id's.
	UIElement* camera_panels[CameraID::COUNT];
	memset(camera_panels, 0, sizeof(UIElement*) * CameraID::COUNT);
	camera_panels[CameraID::Editor_Main]	= camera_panel_bl; 
	camera_panels[CameraID::Editor_OrthoX]	= camera_panel_br; 
	camera_panels[CameraID::Editor_OrthoY]	= camera_panel_tl; 
	camera_panels[CameraID::Editor_OrthoZ]	= camera_panel_tr; 
	
	// Draw camera viewports on top.
	for (int i = 0; i < CameraID::COUNT; i++)
	{
		CameraID::Type	id		= (CameraID::Type)i;
		
		if (id != CameraID::Editor_Main &&
			id != CameraID::Editor_OrthoX &&
			id != CameraID::Editor_OrthoY &&
			id != CameraID::Editor_OrthoZ)
		{
			continue;
		}
		
		EditorCamera*	camera	= static_cast<EditorCamera*>(game->Get_Camera(id));
		UIElement*		panel	= camera_panels[i];

		if (camera == NULL ||
			panel == NULL)
			continue;
		
		Rect screen_box = panel->Get_Screen_Box();

		Rect viewport	= screen_box;
		viewport.X		= viewport.X + 1;
		viewport.Y		= viewport.Y + 1;
		viewport.Width	= viewport.Width - 2;
		viewport.Height	= viewport.Height - 2;

		camera->Set_Viewport(viewport);
		pipeline->Set_Active_Camera(camera);
		pipeline->Draw_Game(time);
	}

	// Set last camera.
	renderer->Clear_Buffer(false, true);
	pipeline->Set_Active_Camera(last_camera);
	
	// Draw all post scene elements.
	UIScene::Post_Draw(time, manager, scene_index);
}

void UIScene_Editor::Recieve_Event(UIEvent e)
{
	switch (e.Type)
	{
	case UIEventType::ToolbarItem_Click:
		if (e.Source->Get_Name() == "pencil_toolbar_item")
			Action_ChangeMode(EditorMode::Pencil);
		if (e.Source->Get_Name() == "rect_toolbar_item")
			Action_ChangeMode(EditorMode::Cube);
		if (e.Source->Get_Name() == "circle_toolbar_item")
			Action_ChangeMode(EditorMode::Sphere);
	
		// CSG operations.
		if (e.Source->Get_Name() == "add_toolbar_item")
			Action_Add();
		if (e.Source->Get_Name() == "subtract_toolbar_item")
			Action_Subtract();

		// Cut/Copy/Paste options.
		if (e.Source->Get_Name() == "cut_toolbar_item")
			Action_Cut();
		if (e.Source->Get_Name() == "copy_toolbar_item")
			Action_Copy();
		if (e.Source->Get_Name() == "paste_toolbar_item")
			Action_Paste();

		break;

	case UIEventType::MenuItem_Click:
		// Nothing to do yet.
		break;
	}
}

void UIScene_Editor::Refresh_State()
{
	// Update toolbar mode states.
	Find_Element<UIToolbarItem*>("pencil_toolbar_item")->Set_Selected	(m_mode == EditorMode::Pencil);
	Find_Element<UIToolbarItem*>("rect_toolbar_item")->Set_Selected		(m_mode == EditorMode::Cube);
	Find_Element<UIToolbarItem*>("circle_toolbar_item")->Set_Selected	(m_mode == EditorMode::Sphere);

	// Dis/Enable csg elements.
	bool csg_enabled = (m_mode == EditorMode::Cube || m_mode == EditorMode::Sphere);
	Find_Element<UIToolbarItem*>("add_toolbar_item")->Set_Enabled		(csg_enabled);
	Find_Element<UIToolbarItem*>("subtract_toolbar_item")->Set_Enabled	(csg_enabled);

	// Dis/Enable cut/copy/paste elements.
	bool cutcopypaste_enabled = (m_mode == EditorMode::Cube || m_mode == EditorMode::Sphere);
	Find_Element<UIToolbarItem*>("cut_toolbar_item")->Set_Enabled		(cutcopypaste_enabled);
	Find_Element<UIToolbarItem*>("copy_toolbar_item")->Set_Enabled		(cutcopypaste_enabled);
	Find_Element<UIToolbarItem*>("paste_toolbar_item")->Set_Enabled		(cutcopypaste_enabled);

	// Update camera states.

	// Update the mode for each editor camera.		
	for (int i = 0; i < CameraID::COUNT; i++)
	{
		if (i != CameraID::Editor_Main &&
			i != CameraID::Editor_OrthoX &&
			i != CameraID::Editor_OrthoY &&
			i != CameraID::Editor_OrthoZ)
			continue;

		EditorCamera* editor_camera = static_cast<EditorCamera*>(Game::Get()->Get_Camera((CameraID::Type)i));
		editor_camera->Set_Editor_Mode(m_mode);
		editor_camera->Set_Editor_Scene(this);
	}
}

void UIScene_Editor::Action_ChangeMode(EditorMode::Type type)
{
	m_mode = type;
	Refresh_State();
}

void UIScene_Editor::Action_Add()
{
}

void UIScene_Editor::Action_Subtract()
{
}

void UIScene_Editor::Action_Cut()
{
}

void UIScene_Editor::Action_Copy()
{
}

void UIScene_Editor::Action_Paste()
{
}

void UIScene_Editor::Update_Tool_Pencil()
{
	Game*			game			= Game::Get();
	ChunkManager*	chunk_manager	= game->Get_Chunk_Manager();
	Input*			input			= Input::Get();
	MouseState*		mouse_state		= input->Get_Mouse_State();
	KeyboardState*	keyboard_state	= input->Get_Keyboard_State();

	bool lshift_down = keyboard_state->Is_Key_Down(KeyboardKey::LeftShift);
	bool lctrl_down = keyboard_state->Is_Key_Down(KeyboardKey::LeftCtrl);
	bool lmouse_down = mouse_state->Was_Button_Clicked(MouseButton::Left);
	bool rmouse_down = mouse_state->Was_Button_Clicked(MouseButton::Right);
	bool mouse_down  = lmouse_down || rmouse_down;

	if (!lshift_down && !lctrl_down)
	{
		if (lmouse_down)
		{
			chunk_manager->Change_Voxel(m_selection_position, m_selection_normal, VoxelType::Solid, Color::Red, false);
		}
		else if (rmouse_down)
		{
			chunk_manager->Change_Voxel(m_selection_position, m_selection_normal, VoxelType::Empty, Color::Red, true);
		}
	}
}

void UIScene_Editor::Update_Tool_Cube()
{
}

void UIScene_Editor::Update_Tool_Sphere()
{
}
