// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_VOXELS_SERIALIZATION_WORLDFILE_
#define _GAME_SCENE_VOXELS_SERIALIZATION_WORLDFILE_

#include "Game\Scene\Voxels\ChunkManagerConfig.h"

class ChunkManager;
class Stream;

class WorldFile
{
public:
	const static int Version = 1;
	const static int Magic		= 0xCAFEDEAD;

	struct Header
	{
		int			magic;
		int			version;
		int			world_hash;
	};

private:
	ChunkManager* m_manager;
	const ChunkManagerConfig& m_config;

	Stream* m_stream;
	Header m_header;

	void Setup_Default_Header();
	bool Validate_Header();

public:
	WorldFile(ChunkManager* manager, const ChunkManagerConfig& config);
	~WorldFile();

	// General properties.
	bool	Open();
	void    Flush();
	void	Close();
	Header	Get_Header();

};

#endif

