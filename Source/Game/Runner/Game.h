// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_MAINGAMERUNNER_
#define _GAME_MAINGAMERUNNER_

#include "Generic\Patterns\Singleton.h"
#include "Engine\Engine\GameRunner.h"

#include "Game\Runner\GameConfig.h"

class Camera;
class ChunkManager;

class Game : public GameRunner, public Singleton<Game>
{
private:
	Camera*			m_camera;
	ChunkManager*	m_chunk_manager;
	GameConfig		m_config;
	const char*		m_config_location;

protected:
	
	// Base functions.
	void Preload();
	void Start();
	void End();
	void Tick(const FrameTime& time);

public:

	// Config settings.
	const GameConfig&		Get_Config();
	const GameEngineConfig& Get_Engine_Config();

	// Constructors.
	Game();
	void Load_Config();

};

#endif

