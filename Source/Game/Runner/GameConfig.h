// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_GAMECONFIG_
#define _GAME_GAMECONFIG_

#include "Engine\Engine\GameEngineConfig.h"
#include "Game\Scene\Voxels\ChunkManagerConfig.h"

class GameConfig
{
public:

	// Engine configuration.
	GameEngineConfig	engine_config;

	// Chunk manager configuration.
	ChunkManagerConfig	chunk_config;

};

#endif