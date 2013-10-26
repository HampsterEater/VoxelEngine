// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef CONFIG_VALUE
#error "CONFIG_VALUE must be defined when including this file!"
#endif

// These define all the different configuration options stored in the ChunkManagerConfig
// class, this file is used to make serialization/deserialization cleaner and easier.

//			Type				Name								Serialized Location

// General sizes.
CONFIG_VALUE(IntVector3,		chunk_size,							"voxels/chunk_size")
CONFIG_VALUE(IntVector3,		region_size,						"voxels/region_size")
CONFIG_VALUE(Vector3,			voxel_size,							"voxels/voxel_size")

// Drawing distances.	
CONFIG_VALUE(IntVector3,		draw_distance,						"voxels/draw_distance")
CONFIG_VALUE(IntVector3,		load_distance,						"voxels/load_distance")
CONFIG_VALUE(IntVector3,		unload_distance,					"voxels/unload_distance")
CONFIG_VALUE(float,				unload_timeout,						"voxels/unload_timeout")

CONFIG_VALUE(IntVector3,		extents_min,						"voxels/extents_min")
CONFIG_VALUE(IntVector3,		extents_max,						"voxels/extents_max")

// Mesh regeneration settings.
CONFIG_VALUE(int,				chunk_regenerations_per_frame,		"voxels/chunk_regenerations_per_frame")

// Chunk management settings.
CONFIG_VALUE(int,				chunk_unloads_per_frame,			"voxels/chunk_unloads_per_frame")
CONFIG_VALUE(int,				chunk_max_load_tasks,				"voxels/max_load_tasks")
CONFIG_VALUE(int,				chunk_max_unload_tasks,				"voxels/max_unload_tasks")
CONFIG_VALUE(int,				chunk_max_regenerate_tasks,			"voxels/max_regenerate_tasks")

// Memory settings. 
// Set to a number above 1, eg. 
//	1.2f will mean allocate 120% of expected max, so we have a 20% buffer.
CONFIG_VALUE(float,				chunk_memory_pool_buffer,			"voxels/chunk_memory_pool_buffer")
CONFIG_VALUE(float,				voxel_memory_pool_buffer,			"voxels/voxel_memory_pool_buffer")

// Map generation settings.
CONFIG_VALUE(int,				map_seed,							"voxels/generation/map_seed")
CONFIG_VALUE(int,				density_threshold,					"voxels/generation/density_threshold")
CONFIG_VALUE(Vector3,			map_terrain_base_noise_sample_step,	"voxels/generation/map_terrain_base_noise_sample_step")
CONFIG_VALUE(float,				map_terrain_base_noise_octaves,		"voxels/generation/map_terrain_base_noise_octaves")
CONFIG_VALUE(float,				map_terrain_base_noise_persistence,	"voxels/generation/map_terrain_base_noise_persistence")
CONFIG_VALUE(float,				map_terrain_base_noise_scale,		"voxels/generation/map_terrain_base_noise_scale")

// Serialization settings.
CONFIG_VALUE(const char*,		 save_directory,					"saving/save_directory")
CONFIG_VALUE(const char*,		 world_file_path,					"saving/world_file_path")
CONFIG_VALUE(const char*,		 region_directory,					"saving/region_directory")
	