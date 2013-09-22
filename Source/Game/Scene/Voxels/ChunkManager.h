// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_VOXELS_CHUNK_MANAGER_
#define _GAME_SCENE_VOXELS_CHUNK_MANAGER_

#include "Engine\Engine\FrameTime.h"

#include "Engine\Renderer\Drawable.h"
#include "Engine\Renderer\Material.h"
#include "Engine\Scene\Tickable.h"

#include "Game\Scene\Voxels\Chunk.h"
#include "Game\Scene\Voxels\ChunkManagerConfig.h"
#include "Game\Scene\Voxels\ChunkLoader.h"
#include "Game\Scene\Voxels\ChunkUnloader.h"
#include "Game\Scene\Voxels\Voxel.h"
#include "Generic\Types\VirtualArray3.h"

#include "Game\Scene\Voxels\Serialization\RegionFile.h"
#include "Game\Scene\Voxels\Serialization\WorldFile.h"

#include "Generic\Threads\Thread.h"
#include "Generic\Threads\Mutex.h"
#include "Generic\Threads\MutexLock.h"
#include "Generic\Threads\Semaphore.h"

#include "Generic\Types\FixedMemoryPool.h"
#include "Generic\Types\IntVector3.h"
#include "Generic\Types\LinkedList.h"

#include <vector>

class Texture;
class TextureAtlas;

class ChunkManager : public Drawable, public Tickable
{
private:
	const ChunkManagerConfig& m_config;

	VirtualArray3<Chunk*>		m_chunks;
	LinkedList<Chunk*>			m_chunk_list;

	IntVector3					m_last_camera_chunk_position;
	Vector3						m_last_camera_position;

	int							m_drawn_voxels;

	int							m_max_chunks;
	int							m_voxels_per_chunk;

	ChunkLoader					m_chunk_loader;
	ChunkUnloader				m_chunk_unloader;
		
	FixedMemoryPool<Chunk>		m_chunk_memory_pool;
	FixedMemoryPool<Voxel>		m_voxel_memory_pool;

	LinkedList<Chunk*>			m_visible_chunks;
	LinkedList<Chunk*>			m_dirty_chunks;

	int							m_seed;

	Texture*					m_voxel_face_atlas_texture;
	TextureAtlas*				m_voxel_face_atlas;
	Material*					m_voxel_face_atlas_material;

	WorldFile					m_world_file;
	VirtualArray3<RegionFile*>	m_region_files_array;
	LinkedList<RegionFile*>		m_region_files;

	Mutex*						m_region_access_mutex;

	// Chunk access. Generally chunk manager should be responsible
	// for creating/setting chunks, so these are private.
	void Queue_Dirty_Chunk(Chunk* chunk);

	// List status updating.
	void Update_Chunk_Lists();
	void Update_Visible_List();
	void Update_Unload_List();
	void Update_Load_List();
	void Regenerate_Dirty_Chunks();

	// List sorting.
	static int Visible_List_Sort_Comparer(Chunk* a, Chunk* b, void* extra);

protected:
	friend class Chunk;
	friend class ChunkLoader;
	friend class ChunkUnloader;

	// Config access.
	const ChunkManagerConfig& Get_Config();

	// Memory pool access.
	FixedMemoryPool<Voxel>& Get_Voxel_Memory_Pool();
	FixedMemoryPool<Chunk>& Get_Chunk_Memory_Pool();
	
	// Resource access.
	TextureAtlas*   Get_Voxel_Face_Atlas();

	// Add / Remove chunks.
	void				Add_Chunk		(IntVector3 position, Chunk* chunk);
	void				Remove_Chunk	(IntVector3 position);
	LinkedList<Chunk*>&	Get_Chunks		();

public:
	ChunkManager(const ChunkManagerConfig& config);
	~ChunkManager();
	
	// Base functions.
	void Draw(const FrameTime& time, RenderPipeline* pipeline);
	void Tick(const FrameTime& time);

	// Chunk access.
	Chunk*		Get_Chunk(IntVector3 position);
	Chunk*		Get_Chunk(int hash);
	Chunk*		Get_Chunk_By_Position(Vector3 position);
	IntVector3  Get_Chunk_Position(Vector3 position);
	IntVector3	Get_Last_Camera_Chunk_Position();
	AABB		Calculate_Chunk_AABB(IntVector3 position);

	// Serialization access.
	WorldFile*  Get_World_File ();
	RegionFile* Get_Region_File(IntVector3 position); // Warning: Blocking!

};

#endif

