// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_VOXELS_VOXEL_
#define _GAME_SCENE_VOXELS_VOXEL_

#include "Engine\Engine\FrameTime.h"
	
// Types of voxels.
struct VoxelType
{
	enum Type
	{
		Empty,
		Solid
	};
};

// Wooooo, an actual voxel struct!
// Everything in this struct is public, it shouldn't really contain any code to act upon
// itself, all code that acts upon it should be in outside classes.
struct Voxel
{
public:
	char Type ;
};

#endif

