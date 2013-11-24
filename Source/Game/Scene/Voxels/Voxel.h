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
		Empty	= 0,
		Solid	= 1
	};
};

// Wooooo, an actual voxel struct!
// Everything in this struct is public, it shouldn't really contain any code to act upon
// itself, all code that acts upon it should be in outside classes.
struct Voxel
{
public:
	unsigned char Type			: 2;	// range: 0-3
	unsigned char ColorIndex	: 6;	// range: 0-63
};

#endif

