// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Engine\GameEngineConfig.h"

void GameEngineConfig::Unpack(const ConfigFile& file)
{
#define CONFIG_VALUE(type, name, serial_name) name = file.Get<type>(serial_name);
	#include "Engine\Engine\GameEngineConfig.inc"
#undef CONFIG_VALUE
}
