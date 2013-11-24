// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\UI\Scenes\UIScene_Game.h"
#include "Engine\UI\Layouts\UILayoutFactory.h"

#include "Engine\Platform\Platform.h"
#include "Engine\Display\Display.h"

#include "Engine\Input\Input.h"

#include "Engine\UI\UIManager.h"

#include "Engine\Renderer\RenderPipeline.h"

#include "Game\UI\Scenes\Editor\UIScene_Editor.h"

#include "Game\Runner\Game.h"

UIScene_Game::UIScene_Game()
{
	Set_Layout("Data/Config/UI/Game.xml");
}

const char* UIScene_Game::Get_Name()
{
	return "UIScene_Game";
}

bool UIScene_Game::Should_Render_Lower_Scenes()
{
	return false;
}

void UIScene_Game::Enter(UIManager* manager)
{
	GameCamera* camera = static_cast<GameCamera*>(Game::Get()->Get_Camera(CameraID::Game));
	camera->Set_Enabled(true);
}	

void UIScene_Game::Exit(UIManager* manager)
{
	GameCamera* camera = static_cast<GameCamera*>(Game::Get()->Get_Camera(CameraID::Game));
	camera->Set_Enabled(false);
}	

void UIScene_Game::Tick(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Switch to editor?
	if (Input::Get()->Get_Keyboard_State()->Was_Key_Pressed(KeyboardKey::F1))
	{
		manager->Push(new UIScene_Editor());
	}

	UIScene::Tick(time, manager, scene_index);
}

void UIScene_Game::Draw(const FrameTime& time, UIManager* manager, int scene_index)
{
	// Render the game camera.
	RenderPipeline::Get()->Set_Active_Camera(Game::Get()->Get_Camera(CameraID::Game));
	RenderPipeline::Get()->Draw_Game(time);

	// Annnnd now all the scene elements.
	RenderPipeline::Get()->Set_Active_Camera(Game::Get()->Get_Camera(CameraID::UI));
	UIScene::Draw(time, manager, scene_index);
}



