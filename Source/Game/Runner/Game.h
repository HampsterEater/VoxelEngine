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

protected:
	
	// Base functions.
	void Start();
	void End();
	void Tick(const FrameTime& time);

public:

	// Config settings.
	const GameConfig& GetConfig();

	// Constructors.
	Game(const GameConfig& config);

};

#endif

