// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_VOXELS_TASKS_CHUNKREGENERATEMESHTASK_
#define _GAME_SCENE_VOXELS_TASKS_CHUNKREGENERATEMESHTASK_

#include "Engine\Tasks\Task.h"

#include "Generic\Types\IntVector3.h"

#include "Game\Scene\Voxels\Generation\ChunkGenerator.h"

class Chunk;
class ChunkManager;

class ChunkRegenerateMeshTask : public Task
{
private:	
	ChunkManager*				m_manager;
	Chunk*						m_chunk;
	const ChunkManagerConfig&	m_config;

public:
	ChunkRegenerateMeshTask(ChunkManager* manager, Chunk* chunk, const ChunkManagerConfig& config);

	Chunk*	   Get_Chunk();
	 
	void Run();

};

#endif

