// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_UI_SCENES_UISCENE_GAME_
#define _GAME_UI_SCENES_UISCENE_GAME_

#include "Engine\UI\UIScene.h"

class UIScene_Game : public UIScene
{
private:

protected:

public:
	UIScene_Game();

	const char* Get_Name();
	bool Should_Render_Lower_Scenes();
	
	void Enter(UIManager* manager);
	void Exit(UIManager* manager);
	
	void Tick(const FrameTime& time, UIManager* manager, int scene_index);
	void Draw(const FrameTime& time, UIManager* manager, int scene_index);

};

#endif

