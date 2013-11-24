// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MAINGAMERUNNER_
#define _GAME_MAINGAMERUNNER_

#include "Generic\Patterns\Singleton.h"
#include "Engine\Engine\GameRunner.h"

#include "Game\Runner\GameConfig.h"

#include "Engine\Engine\GameRunner.h"

#include "Game\Scene\Cameras\GameCamera.h"
#include "Game\Scene\Cameras\EditorCamera.h"

class Camera;
class GameCamera;
class EditorCamera;
class ChunkManager;
class Light;
class FontHandle;
class FontRenderer;
class EditorGrid;
class EditorSelectionPrimitive;

struct CameraID
{
	enum Type
	{
		Game = 0,
		UI,
		Editor_Main,
		Editor_OrthoX,
		Editor_OrthoY,
		Editor_OrthoZ,
		COUNT
	};
};

// Required function to integrate with editor.
void		Print_Game_Version();
GameRunner* New_Game();

class Game : public GameRunner, public Singleton<Game>
{
private:
	ChunkManager*				m_chunk_manager;
	GameConfig					m_config;
	const char*					m_config_location;

	Light*						m_directional_light;
			
	Camera*						m_cameras[CameraID::COUNT];

	EditorGrid*					m_editor_projection_grid;
	EditorGrid*					m_editor_orthox_grid;
	EditorGrid*					m_editor_orthoy_grid;
	EditorGrid*					m_editor_orthoz_grid;

	EditorSelectionPrimitive*	m_editor_mouse_selection_primitive;
	EditorSelectionPrimitive*	m_editor_main_selection_primitive;

protected:
	
	// Base functions.
	void Preload();
	void Start();
	void End();
	void Tick(const FrameTime& time);

	// Initialisation
	void Setup_Editor();
	void Setup_Cameras();

public:

	// Config settings.
	const GameConfig&		Get_Config();
	const GameEngineConfig& Get_Engine_Config();
	ChunkManager*			Get_Chunk_Manager();

	// Editor stuff.
	EditorSelectionPrimitive* Get_Editor_Mouse_Selection_Primitive();
	EditorSelectionPrimitive* Get_Editor_Main_Selection_Primitive();

	// Cameras.
	Camera*					Get_Camera(CameraID::Type camera);

	// Constructors.
	Game();
	void Load_Config();

};

#endif

