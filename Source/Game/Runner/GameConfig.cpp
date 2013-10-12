// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Runner\GameConfig.h"

void GameConfig::Unpack(ConfigFile& file)
{
	engine_config.Unpack(file);
	chunk_config.Unpack(file);
}