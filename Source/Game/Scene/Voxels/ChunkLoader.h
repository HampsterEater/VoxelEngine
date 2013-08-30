// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_VOXELS_CHUNK_LOADER_
#define _GAME_SCENE_VOXELS_CHUNK_LOADER_

#include "Game\Scene\Voxels\ChunkManagerConfig.h"
#include "Game\Scene\Voxels\Generation\ChunkGenerator.h"

#include "Generic\Types\VirtualArray3.h"
#include "Generic\Types\IntVector3.h"
#include "Generic\Types\LinkedList.h"

#include <vector>

class Thread;
class Semaphore;
class Chunk;
class ChunkManager;
class Mutex;

// How much the Y axis is biased. The higher this is the more we perfer to load
// chunks on the cameras "level"
#define CHUNK_LOAD_Y_BIAS 2

class ChunkLoader
{
private:
	const ChunkManagerConfig&	m_config;
	
	ChunkManager*				m_manager;

	Thread*						m_thread;
	Semaphore*					m_semaphore;
	Semaphore*					m_pause_semaphore;

	bool						m_running;
	bool						m_paused;
	bool						m_refresh_queue;

	LinkedList<IntVector3>		m_chunks;

	LinkedList<Chunk*>			m_loaded_chunks;
	LinkedList<IntVector3>		m_loaded_chunk_positions;
	Mutex*						m_loaded_chunks_mutex;

	ChunkGenerator				m_generator;

protected:
	friend class Chunk;
	friend class ChunkManager;

	static void Static_Chunk_Load_Thread(Thread* thread, void* ptr);
	void Chunk_Load_Thread(Thread* thread);

	void Load_Chunk(Chunk* chunk);
	
	Mutex* Get_Mutex();

	void Refresh_Queue();

	void Pause();
	void Resume();

public:
	ChunkLoader(ChunkManager* manager, const ChunkManagerConfig& config);
	~ChunkLoader();
	
	Chunk* Consume_Chunk();

	void Refresh();

};

#endif

