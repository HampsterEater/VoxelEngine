// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_VOXELS_SERIALIZATION_REGIONFILE_
#define _GAME_SCENE_VOXELS_SERIALIZATION_REGIONFILE_

#include "Game\Scene\Voxels\ChunkManagerConfig.h"

#include "Generic\Threads\Mutex.h"
#include "Generic\Threads\MutexLock.h"

class ChunkManager;
class Stream;
class Chunk;

class RegionFile
{
public:
	const static int Version	= 1;
	const static int Magic		= 0xDEADCAFE;

	struct Header
	{
		int magic;
		int version;		
		int world_hash;
	};
	
private:
	ChunkManager* m_manager;
	const ChunkManagerConfig& m_config;
	IntVector3 m_position;
	IntVector3 m_chunk_origin;
	int m_chunk_data_size;
	int m_voxel_data_offset;

	Mutex* m_access_mutex;

	Stream* m_stream;
	Header m_header;

	bool* m_chunk_exists;
	int m_chunk_count;

	void Setup_Default_Header();
	bool Validate_Header();
	
	__forceinline int Flatten_Index(int x, int y, int z)
	{
		return y + m_config.region_size.Y * (x + m_config.region_size.Z * z);
	}

public:
	RegionFile(IntVector3 position, ChunkManager* manager, const ChunkManagerConfig& config);
	~RegionFile();

	// General properties.
	bool	Open		();
	void    Flush		();
	void	Close		();
	Header	Get_Header	();
	
	bool	Contains_Chunk	(Chunk* chunk);
	void	Save_Chunk		(Chunk* chunk);
	void	Load_Chunk		(Chunk* chunk);

};

#endif

