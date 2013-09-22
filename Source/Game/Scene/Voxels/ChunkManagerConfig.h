// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_CHUNKMANAGERCONFIG_
#define _GAME_CHUNKMANAGERCONFIG_

#include "Generic\Types\Vector3.h"
#include "Generic\Types\IntVector3.h"

#include "Engine\Config\ConfigFile.h"

class ChunkManagerConfig : public ConfigFile
{
public:

	// Include settings.
#define CONFIG_VALUE(type, name, serial_name) type name;
	#include "Game\Scene\Voxels\ChunkManagerConfig.inc"
#undef CONFIG_VALUE

	// Loading methods.
	void Unpack(const ConfigFile& file);

};

#endif