// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Voxels\ChunkUnloader.h"

#include "Game\Scene\Voxels\ChunkManager.h"

#include "Engine\Engine\FrameTime.h"

#include "Generic\Threads\Thread.h"
#include "Generic\Threads\Mutex.h"
#include "Generic\Threads\MutexLock.h"
#include "Generic\Threads\Semaphore.h"

ChunkUnloader::ChunkUnloader(ChunkManager* manager, const ChunkManagerConfig& config)
	: m_config(config)
	, m_manager(manager)
	, m_running(true)
	, m_paused(false)
	, m_refresh_queue(false)
{
	// Create work semaphore.
	m_semaphore = Semaphore::Create();
	DBG_ASSERT(m_semaphore != NULL);
	
	// Create pause semaphore.
	m_pause_semaphore = Semaphore::Create();
	DBG_ASSERT(m_pause_semaphore != NULL);
	
	// Create produced chunks mutex.
	m_unloaded_chunks_mutex = Mutex::Create();
	DBG_ASSERT(m_unloaded_chunks_mutex != NULL);

	// Create load/save thread.
	m_thread = Thread::Create(Static_Chunk_Unload_Thread, this);
	DBG_ASSERT(m_thread != NULL);
	
	// Boot up threads.
	//m_thread->Set_Affinity(1 << (1 % m_thread->Get_Core_Count()));
	//m_thread->Set_Priority(ThreadPriority::Low);
	m_thread->Start();
}

ChunkUnloader::~ChunkUnloader()
{
	// Wait for threads to shut down.
	m_running = false;
	while (m_thread->Is_Running())
	{
		Thread::Get_Current()->Sleep(0.01f);
	}

	// Delete thread stuff.
	SAFE_DELETE(m_thread);
	SAFE_DELETE(m_semaphore);
	SAFE_DELETE(m_pause_semaphore);
	SAFE_DELETE(m_unloaded_chunks_mutex);
}

Mutex* ChunkUnloader::Get_Mutex()
{
	return m_unloaded_chunks_mutex;
}

void ChunkUnloader::Pause()
{
	DBG_ASSERT(!m_paused);

	m_paused = true;
	m_semaphore->Signal();
}

void ChunkUnloader::Resume()
{
	m_paused = false;
	m_pause_semaphore->Signal();
}

void ChunkUnloader::Refresh()
{
	m_refresh_queue = true;
	m_semaphore->Signal();
}

void ChunkUnloader::Refresh_Queue()
{
	IntVector3			camera_chunk = m_manager->Get_Last_Camera_Chunk_Position();
	LinkedList<Chunk*>& chunks		 = m_manager->Get_Chunks();

	// Clear current chunk list.
	m_chunks.Clear();

	// Look for all chunks further than the unload distance.
	for (LinkedList<Chunk*>::Iterator iter = chunks.Begin(); iter != chunks.End(); iter++)
	{
		Chunk* chunk = *iter;
		IntVector3 chunk_position = chunk->Get_Position();

		if (chunk->Get_Status() != ChunkStatus::Loaded)
		{
			continue;
		}

		int distance_x = abs(chunk_position.X - camera_chunk.X);
		int distance_y = abs(chunk_position.Y - camera_chunk.Y);
		int distance_z = abs(chunk_position.Z - camera_chunk.Z);

		if (distance_x >= m_config.unload_distance.X ||
			distance_y >= m_config.unload_distance.Y ||
			distance_z >= m_config.unload_distance.Z)
		{
			if (chunk->Get_Unload_Timer() >= m_config.unload_timeout)
			{	
				m_chunks.Add(chunk_position);
			}
		}
		else
		{
			chunk->Reset_Unload_Timer();
		}
	}
}

void ChunkUnloader::Static_Chunk_Unload_Thread(Thread* thread, void* ptr)
{
	ChunkUnloader* loader = reinterpret_cast<ChunkUnloader*>(ptr);
	loader->Chunk_Unload_Thread(thread);
}

void ChunkUnloader::Chunk_Unload_Thread(Thread* thread)
{
	while (m_running)
	{
		// Wait for work!
		m_semaphore->Wait();

		// Are we paused? If so wait for resume semaphore.
		if (m_paused == true)
		{
			m_pause_semaphore->Wait();
		}

		while (m_chunks.Size() > 0 ||
			   m_refresh_queue == true)
		{
			// Do we need to refresh our load list?
			if (m_refresh_queue == true)
			{
				Refresh_Queue();
				m_refresh_queue = false;
			}

			// Do we need to update our queue?
			IntVector3 closest_chunk = IntVector3(0, 0, 0);
			bool found_chunk = false;

			// Find furthest chunk to camera, cos thats what we will load!		
			const LinkedList<IntVector3>::Node* closest_chunk_node		= NULL;
			float								closest_distance		= 0.0f;
		
			IntVector3							camera_position			= m_manager->Get_Last_Camera_Chunk_Position();
		
			// Find Furthest chunk to unload.
			for (LinkedList<IntVector3>::Iterator iter = m_chunks.Begin(); iter != m_chunks.End(); iter++)
			{
				IntVector3 chunk = *iter;
				float distance = (chunk - camera_position).Length_Squared();

				if (found_chunk == false || distance > closest_distance)
				{
					closest_chunk = chunk;
					closest_distance = distance;
					closest_chunk_node = iter.Get_Node();
					found_chunk = true;
				}
			}
			
			// Start generating!
			if (found_chunk == true)
			{
				m_chunks.Remove(closest_chunk_node);
					
				Chunk* chunk = m_manager->Get_Chunk(closest_chunk);

				chunk->Set_Status(ChunkStatus::Unloading);
		
				// Load actual chunk.
				Unload_Chunk(chunk);
			
				// Loaded!
				chunk->Set_Status(ChunkStatus::Unloaded);

				// Use consumer.
				{
					MutexLock lock(m_unloaded_chunks_mutex);
					m_unloaded_chunks.Add(chunk);
					m_unloaded_chunk_positions.Add(closest_chunk);
				}
			}
		}
	}
}

void ChunkUnloader::Unload_Chunk(Chunk* chunk)
{
	IntVector3 position = chunk->Get_Position();

	RegionFile* region = m_manager->Get_Region_File(chunk->Get_Region());
	region->Save_Chunk(chunk);

	//DBG_LOG("Unloading chunk: %i,%i,%i", position.X, position.Y, position.Z);
}

Chunk* ChunkUnloader::Consume_Chunk()
{
	if (m_unloaded_chunks.Size() > 0)
	{
		Chunk* c = m_unloaded_chunks.Pop_First();
		m_unloaded_chunk_positions.Pop_First();

		return c;
	}

	return NULL;
}