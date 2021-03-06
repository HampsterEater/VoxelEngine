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
#include "Game\Scene\Voxels\Voxel.h"
#include "Generic\Types\VirtualArray3.h"

#include "Game\Scene\Voxels\Tasks\ChunkLoadTask.h"
#include "Game\Scene\Voxels\Tasks\ChunkUnloadTask.h"
#include "Game\Scene\Voxels\Tasks\ChunkRegenerateMeshTask.h"

#include "Game\Scene\Voxels\Serialization\RegionFile.h"
#include "Game\Scene\Voxels\Serialization\WorldFile.h"

#include "Game\Scene\Voxels\Generation\ChunkGenerator.h"

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

// How much the Y axis is biased. The higher this is the more we perfer to load
// chunks on the cameras "level"
#define CHUNK_LOAD_Y_BIAS 2

struct ChunkRaycastResult
{
	Chunk*			Chunk;
	IntVector3		RelativeVoxel;
	IntVector3		AbsoluteVoxel;
	IntVector3		Normal;
};

struct ChunkRaycastBoundries
{
	bool  Use_YBoundry;
	float YBoundry;

	bool  Use_XBoundry;
	float XBoundry;

	bool  Use_ZBoundry;
	float ZBoundry;

	ChunkRaycastBoundries()
		: Use_XBoundry(false)
		, Use_YBoundry(false)
		, Use_ZBoundry(false)
	{
	}
};

class ChunkManager : public Drawable, public Tickable
{
private:
	const ChunkManagerConfig&				m_config;

	VirtualArray3<Chunk*>					m_chunks;
	LinkedList<Chunk*>						m_chunk_list;

	IntVector3								m_last_camera_chunk_position;
	Vector3									m_last_camera_position;

	int										m_drawn_voxels;

	int										m_max_chunks;
	int										m_voxels_per_chunk;

	FixedMemoryPool<Chunk>					m_chunk_memory_pool;
	FixedMemoryPool<Voxel>					m_voxel_memory_pool;

	LinkedList<ChunkLoadTask*>				m_load_tasks;
	LinkedList<ChunkUnloadTask*>			m_unload_tasks;
	LinkedList<ChunkRegenerateMeshTask*>	m_regenerate_mesh_tasks;	

	LinkedList<IntVector3>					m_chunk_unload_list;
	LinkedList<IntVector3>					m_chunk_load_list;

	LinkedList<Chunk*>						m_visible_chunks;
	LinkedList<Chunk*>						m_dirty_chunks;

	int										m_seed;

	TextureHandle*							m_voxel_face_atlas_texture;
	TextureAtlas*							m_voxel_face_atlas;
	Material*								m_voxel_face_atlas_material;

	WorldFile*								m_world_file;
	VirtualArray3<RegionFile*>				m_region_files_array;
	LinkedList<RegionFile*>					m_region_files;

	Mutex*									m_region_access_mutex;

	bool									m_dirty_chunks_sorted;

	// Chunk access. Generally chunk manager should be responsible
	// for creating/setting chunks, so these are private.
	void Queue_Dirty_Chunk(Chunk* chunk);

	// List status updating.
	void Update_Chunk_Lists		();
	void Update_Visible_List	();
	void Update_Unload_List		();
	void Update_Load_List		();
	void Regenerate_Dirty_Chunks();

	void Create_New_Tasks		();
	void Poll_Running_Tasks		();

	// List sorting.
	static int Visible_List_Sort_Comparer(Chunk* a, Chunk* b, void* extra);
	static int Chunk_List_Sort_Comparer(IntVector3 a, IntVector3 b, void* extra);

protected:
	friend class Chunk;
	friend class ChunkLoadTask;
	friend class ChunkUnloadTask;

	// Config access.
	const ChunkManagerConfig& Get_Config();
	ChunkGenerator* Get_Chunk_Generator();

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
	IntVector3	Get_Absolute_Voxel_Position(Vector3 position);
	IntVector3	Get_Relative_Voxel_Position(Vector3 position);
	void		Get_Chunk_By_Absolute_Voxel_Position(IntVector3 absolute_position, Chunk** chunk, IntVector3* relative_position);

	// Serialization access.
	WorldFile*  Get_World_File ();
	RegionFile* Get_Region_File(IntVector3 position); // Warning: Blocking!

	// Raycasting functionality.
	int Raycast(Vector3 start, Vector3 end, std::vector<ChunkRaycastResult>& results, int max_hits = 0, ChunkRaycastBoundries* boundries = NULL);
	int RaycastIntBound(float s, float ds);
	int RaycastMod(int value, int modulus);
	
	// Voxel modification.
	void Change_Voxel(IntVector3 absolute_position, IntVector3 face_normal, VoxelType::Type type, Color color, bool overwrite);

};

#endif

