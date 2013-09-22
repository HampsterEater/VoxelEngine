// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_GAMEENGINECONFIG_
#define _ENGINE_GAMEENGINECONFIG_

#include "Engine\Config\ConfigFile.h"

class GameEngineConfig : public ConfigFile
{
public:

	// Include settings.
#define CONFIG_VALUE(type, name, serial_name) type name;
	#include "Engine\Engine\GameEngineConfig.inc"
#undef CONFIG_VALUE

	// Loading methods.
	void Unpack	(const ConfigFile& file);

};

#endif