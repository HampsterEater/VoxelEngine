// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Voxels\ChunkManagerConfig.h"

void ChunkManagerConfig::Unpack(const ConfigFile& file)
{
#define CONFIG_VALUE(type, name, serial_name) name = file.Get<type>(serial_name);
	#include "Game\Scene\Voxels\ChunkManagerConfig.inc"
#undef CONFIG_VALUE
}
