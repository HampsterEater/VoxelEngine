// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_VOXELS_CHUNK_UNLOADER_
#define _GAME_SCENE_VOXELS_CHUNK_UNLOADER_

#include "Game\Scene\Voxels\ChunkManagerConfig.h"

#include "Generic\Types\VirtualArray3.h"
#include "Generic\Types\IntVector3.h"
#include "Generic\Types\LinkedList.h"

#include <vector>

class Thread;
class Semaphore;
class Chunk;
class ChunkManager;
class Mutex;

class ChunkUnloader
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
	Mutex*						m_chunks_mutex;
	
	LinkedList<Chunk*>			m_unloaded_chunks;
	LinkedList<IntVector3>		m_unloaded_chunk_positions;
	Mutex*						m_unloaded_chunks_mutex;

protected:
	friend class Chunk;
	friend class ChunkManager;

	static void Static_Chunk_Unload_Thread(Thread* thread, void* ptr);
	void Chunk_Unload_Thread(Thread* thread);

	void Unload_Chunk(Chunk* chunk);
	
	Mutex* Get_Mutex();

	void Refresh_Queue();

	void Pause();
	void Resume();

public:
	ChunkUnloader(ChunkManager* manager, const ChunkManagerConfig& config);
	~ChunkUnloader();
	
	Chunk* Consume_Chunk();

	void Refresh();

};

#endif

