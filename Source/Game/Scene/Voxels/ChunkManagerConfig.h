// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_CHUNKMANAGERCONFIG_
#define _GAME_CHUNKMANAGERCONFIG_

#include "Generic\Types\Vector3.h"
#include "Generic\Types\IntVector3.h"

class ChunkManagerConfig
{
public:

	// General sizes.
	IntVector3 chunk_size;
	Vector3	   voxel_size;

	// Drawing distances.
	IntVector3 draw_distance;
	IntVector3 load_distance;
	IntVector3 unload_distance;
	float	   unload_timeout;

	// Mesh regeneration settings.
	int chunk_regenerations_per_frame;

	// Chunk management settings.
	int chunk_unloads_per_frame;
	
	// Memory settings. 
	// Set to a number above 1, eg. 
	//	1.2f will mean allocate 120% of expected max, so we have a 20% buffer.
	float chunk_memory_pool_buffer;
	float voxel_memory_pool_buffer;

	// Map generation settings.
	int			map_seed;
	int			density_threshold;
	Vector3		map_terrain_base_noise_sample_step;
	float		map_terrain_base_noise_octaves;
	float		map_terrain_base_noise_persistence;
	float		map_terrain_base_noise_scale;

};

#endif