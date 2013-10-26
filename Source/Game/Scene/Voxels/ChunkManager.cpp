// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Voxels\ChunkManager.h"
#include "Game\Scene\Voxels\Generation\ChunkGenerator.h"
#include "Engine\Scene\Camera.h"
#include "Generic\Math\Math.h"
#include "Generic\Types\LinkedList.h"
#include "Generic\Math\Random.h"
#include "Engine\Platform\Platform.h"
#include "Engine\Engine\GameEngine.h"
#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Textures\TextureAtlas.h"
#include "Engine\Renderer\RenderPipeline.h"

#include "Engine\Tasks\TaskManager.h"

#include "Engine\Renderer\Shaders\Shader.h"
#include "Engine\Renderer\Shaders\ShaderFactory.h"
#include "Engine\Renderer\Shaders\ShaderProgram.h"

#include <algorithm>

ChunkManager::ChunkManager(const ChunkManagerConfig& config)
	: m_config(config)
	, m_last_camera_chunk_position(-9999, -9999, -9999)
	, m_last_camera_position(-9999, -9999, -9999)
	, m_max_chunks((m_config.unload_distance.X * 2) * 
				   (m_config.unload_distance.Y * 2) * 
				   (m_config.unload_distance.Z * 2))
	, m_voxels_per_chunk((m_config.chunk_size.X) * 
						 (m_config.chunk_size.Y) * 
						 (m_config.chunk_size.Z))
	, m_chunk_memory_pool(m_max_chunks * config.chunk_memory_pool_buffer)
	, m_voxel_memory_pool(m_max_chunks * config.voxel_memory_pool_buffer, sizeof(Voxel) * m_voxels_per_chunk)
	, m_world_file(new WorldFile(this, config))
{
	m_voxel_face_atlas_texture = TextureFactory::Load("Data/Textures/Chunks/Voxel_Face_Atlas.png", (TextureFlags::Type)0);
	m_voxel_face_atlas = new TextureAtlas(m_voxel_face_atlas_texture, 16, 16);
	m_voxel_face_atlas_material = new Material(m_voxel_face_atlas_texture, 0.0f, Vector3(0, 0, 0));

	// Access mutexes!
	m_region_access_mutex = Mutex::Create();
	DBG_ASSERT(m_region_access_mutex != NULL);

	// Open the world file!
	bool ret = m_world_file->Open();
	DBG_ASSERT(ret == true);
}

ChunkManager::~ChunkManager()
{
	m_world_file->Close();
	SAFE_DELETE(m_world_file);

	for (LinkedList<RegionFile*>::Iterator iter = m_region_files.Begin(); iter != m_region_files.End(); iter++)
	{
		RegionFile* region = *iter;
		region->Close();

		SAFE_DELETE(region);
	}
	m_region_files.Clear();
	
	for (LinkedList<ChunkLoadTask*>::Iterator iter = m_load_tasks.Begin(); iter != m_load_tasks.End(); iter++)
	{
		ChunkLoadTask* task = *iter;
		SAFE_DELETE(task);
	}
	m_load_tasks.Clear();
	
	for (LinkedList<ChunkUnloadTask*>::Iterator iter = m_unload_tasks.Begin(); iter != m_unload_tasks.End(); iter++)
	{
		ChunkUnloadTask* task = *iter;
		SAFE_DELETE(task);
	}
	m_unload_tasks.Clear();
	
	for (LinkedList<ChunkRegenerateMeshTask*>::Iterator iter = m_regenerate_mesh_tasks.Begin(); 
			iter != m_regenerate_mesh_tasks.End();
			iter++)
	{
		ChunkRegenerateMeshTask* task = *iter;
		SAFE_DELETE(task);
	}
	m_regenerate_mesh_tasks.Clear();

	SAFE_DELETE(m_region_access_mutex);
	SAFE_DELETE(m_voxel_face_atlas);
	//SAFE_DELETE(m_voxel_face_atlas_texture);	
	SAFE_DELETE(m_voxel_face_atlas_material);
}

const ChunkManagerConfig& ChunkManager::Get_Config()
{
	return m_config;
}

TextureAtlas* ChunkManager::Get_Voxel_Face_Atlas()
{
	return m_voxel_face_atlas;
}

void ChunkManager::Tick(const FrameTime& time)
{
	// Do we have a camera?
	Renderer* renderer	= Renderer::Get();
	Camera* camera		= GameEngine::Get()->Get_RenderPipeline()->Get_Active_Camera();
	if (camera == NULL)
	{
		return;
	}

	// If we have changed chunk, update queues.
	Vector3	   position		  = camera->Get_Position();
	IntVector3 chunk_position = Get_Chunk_Position(position);

	if (m_last_camera_chunk_position != chunk_position)
	{
		m_last_camera_position		 = position;
		m_last_camera_chunk_position = chunk_position;

		// Update the unload/load/visible lists!
		//float t = Platform::Get()->Get_Ticks();
		Update_Chunk_Lists();
		//float elapsed = Platform::Get()->Get_Ticks() - t;
		//printf("Took %f ms\n", elapsed);
	}

	// Create new tasks.
	Create_New_Tasks();

	// Update running tasks.
	Poll_Running_Tasks();

	// Regenerate dirty chunk meshes.
	Regenerate_Dirty_Chunks();
}

void ChunkManager::Create_New_Tasks()
{
	TaskManager* task_manager = TaskManager::Get();
	
	IntVector3 camera_position = Get_Last_Camera_Chunk_Position();
		
	// Create unload tasks.
	while (m_unload_tasks.Size() < m_config.chunk_max_unload_tasks &&
		   m_chunk_unload_list.Size() > 0)
	{
		IntVector3 chunk = m_chunk_unload_list.Pop_First();
			
		ChunkUnloadTask* task = new ChunkUnloadTask(this, chunk);
		
		TaskID id = task_manager->Add_Task(task);
		task_manager->Queue_Task(id);

		m_unload_tasks.Add(task);
	}

	// Create load tasks.
	while (m_load_tasks.Size() < m_config.chunk_max_load_tasks &&
		   m_chunk_load_list.Size() > 0)
	{
		IntVector3 chunk = m_chunk_load_list.Pop_First();

		ChunkLoadTask* task = new ChunkLoadTask(this, chunk, m_config);
		
		TaskID id = task_manager->Add_Task(task);
		task_manager->Queue_Task(id);

		m_load_tasks.Add(task);
	}
}

void ChunkManager::Poll_Running_Tasks()
{
	Renderer* renderer = Renderer::Get();

	// Remove newly unloaded chunks.
	int counter = 0;		
	for (LinkedList<ChunkUnloadTask*>::Iterator iter = m_unload_tasks.Begin(); 
			iter != m_unload_tasks.End() && counter++ < m_config.chunk_unloads_per_frame;
			iter++)
	{
		ChunkUnloadTask* task = *iter;
		if (task->Is_Completed() == false)
		{
			continue;
		}

		Chunk* chunk = task->Get_Chunk();
		Remove_Chunk(chunk->Get_Position());
		m_chunk_memory_pool.Release(chunk);	

		iter = m_unload_tasks.Remove(iter);

		SAFE_DELETE(task);
	}

	// Insert newly loaded chunks.
	for (LinkedList<ChunkLoadTask*>::Iterator iter = m_load_tasks.Begin(); 
			iter != m_load_tasks.End();
			iter++)
	{
		ChunkLoadTask* task = *iter;
		if (task->Is_Completed() == false)
		{
			continue;
		}

		Chunk* chunk = task->Get_Chunk();
		Add_Chunk(chunk->Get_Position(), chunk);
		chunk->Mark_Dirty(true);

		iter = m_load_tasks.Remove(iter);

		SAFE_DELETE(task);
	}

	// Insert regenerated chunks into opengl!
	counter = 0;
	for (LinkedList<ChunkRegenerateMeshTask*>::Iterator iter = m_regenerate_mesh_tasks.Begin(); 
			iter != m_regenerate_mesh_tasks.End() && counter++ < m_config.chunk_regenerations_per_frame;
			iter++)
	{
		ChunkRegenerateMeshTask* task = *iter;
		if (task->Is_Completed() == false)
		{
			continue;
		}

		Chunk* chunk = task->Get_Chunk();
		chunk->Regenerate_Mesh(renderer, false);
		chunk->Set_Regenerating(false);

		iter = m_regenerate_mesh_tasks.Remove(iter);
		SAFE_DELETE(task);
	}
}

void ChunkManager::Update_Unload_List()
{
	IntVector3 camera_chunk = Get_Last_Camera_Chunk_Position();

	// Clear current chunk list.
	m_chunk_unload_list.Clear();

	// Look for all chunks further than the unload distance.
	for (LinkedList<Chunk*>::Iterator iter = m_chunk_list.Begin(); iter != m_chunk_list.End(); iter++)
	{
		Chunk* chunk = *iter;
		IntVector3 chunk_position = chunk->Get_Position();

		if (chunk->Get_Status() != ChunkStatus::Loaded)
		{
			continue;
		}

		if (chunk->Is_Regenerating() == true)
		{
			continue;
		}

		// Calculate distance to chunk.
		int distance_x = abs(chunk_position.X - camera_chunk.X);
		int distance_y = abs(chunk_position.Y - camera_chunk.Y);
		int distance_z = abs(chunk_position.Z - camera_chunk.Z);

		if (distance_x >= m_config.unload_distance.X ||
			distance_y >= m_config.unload_distance.Y ||
			distance_z >= m_config.unload_distance.Z)
		{
			if (chunk->Get_Unload_Timer() >= m_config.unload_timeout)
			{
				// Don't unload a chunk thats already unloading!		
				bool found = false;
				for (LinkedList<ChunkUnloadTask*>::Iterator iter = m_unload_tasks.Begin(); iter != m_unload_tasks.End(); iter++)
				{
					ChunkUnloadTask* task = *iter;
					if (task->Get_Chunk_Position() == chunk_position)
					{
						found = true;
						break;
					}
				}
				for (LinkedList<ChunkRegenerateMeshTask*>::Iterator iter = m_regenerate_mesh_tasks.Begin(); iter != m_regenerate_mesh_tasks.End(); iter++)
				{
					ChunkRegenerateMeshTask* task = *iter;
					if (task->Get_Chunk()->Get_Position() == chunk_position)
					{
						found = true;
						break;
					}
				}
				if (found == true)
				{
					continue;
				}

				m_chunk_unload_list.Add(chunk_position);
			}
		}
		else
		{
			chunk->Reset_Unload_Timer();
		}
	}

	// Sort load list so the closest is first.	
	m_chunk_unload_list.Sort(Chunk_List_Sort_Comparer, true, &m_last_camera_chunk_position);
}

void ChunkManager::Update_Load_List()
{
	IntVector3 camera_chunk = Get_Last_Camera_Chunk_Position();

	// Clear current chunk list.
	m_chunk_load_list.Clear();

	// Look for all chunks in our load distance.
	for (int x = camera_chunk.X - m_config.load_distance.X; x < camera_chunk.X + m_config.load_distance.X; x++)
	{
		for (int y = camera_chunk.Y - m_config.load_distance.Y; y < camera_chunk.Y + m_config.load_distance.Y; y++)
		{
			for (int z = camera_chunk.Z - m_config.load_distance.Z; z < camera_chunk.Z + m_config.load_distance.Z; z++)
			{
				// Check chunk is in extents.
				if ((m_config.extents_min.X != 0 && x < m_config.extents_min.X) ||
					(m_config.extents_max.X != 0 && x > m_config.extents_max.X) ||
					(m_config.extents_min.Y != 0 && y < m_config.extents_min.Y) ||
					(m_config.extents_max.Y != 0 && y > m_config.extents_max.Y) ||
					(m_config.extents_min.Z != 0 && z < m_config.extents_min.Z) ||
					(m_config.extents_max.Z != 0 && z > m_config.extents_max.Z))
					continue;

				// Load chunk info!
				IntVector3 chunk_position = IntVector3(x, y, z);
				Chunk*	   chunk		  = Get_Chunk(chunk_position); 

				if (chunk != NULL)
				{
					continue;
				}
				
				// Don't load a chunk thats already loading!		
				bool found = false;
				for (LinkedList<ChunkLoadTask*>::Iterator iter = m_load_tasks.Begin(); iter != m_load_tasks.End(); iter++)
				{
					ChunkLoadTask* task = *iter;
					if (task->Get_Chunk_Position() == chunk_position)
					{
						found = true;
						break;
					}
				}
				if (found == true)
				{
					continue;
				}

				// No chunk loaded? Time to load!
				m_chunk_load_list.Add(chunk_position);
			}
		}
	}

	// Sort load list so the closest is first.	
	m_chunk_load_list.Sort(Chunk_List_Sort_Comparer, false, &m_last_camera_chunk_position);
}

void ChunkManager::Draw(const FrameTime& time, RenderPipeline* pipeline)
{
	// Reset stats.
	m_drawn_voxels = 0;

	// Camera position.
	Renderer* renderer = Renderer::Get();
	Frustum frustum	= pipeline->Get_Active_Camera()->Get_Frustum();
	
	// Bind terrain texture.
	renderer->Bind_Material(m_voxel_face_atlas_material);

	// Update shader uniforms.	
	pipeline->Update_Shader_Uniforms();
	
	// Render all visible chunks.
	for (LinkedList<Chunk*>::Iterator iter = m_visible_chunks.Begin(); iter != m_visible_chunks.End(); iter++)
	{
		Chunk* chunk = *iter;

		if (chunk->Get_Status() == ChunkStatus::Loaded &&
			chunk->Is_Empty() == false &&
			chunk->Should_Render() == true)
		{
			Sphere chunk_sphere = chunk->Get_Bounding_Sphere();
			if (frustum.Intersects(chunk_sphere) != Frustum::IntersectionResult::Outside)
			{
				chunk->Draw(time, pipeline);
				m_drawn_voxels += chunk->Drawn_Voxels();
			}
		}
	}
	
	// Emit stats.
// DEBUG =======================================================================================
	static float s_emit_fps_time = 0.0f;
	s_emit_fps_time += time.Get_Delta();
	if (s_emit_fps_time > 60.0f)// || time.Get_Frame_Time() > 10.0f)
	{
		printf("[FPS %i, FRAME TIME %0.2f, UPDATE TIME %0.2f, DRAW TIME %0.2f, VOXELS %i, CHUNKS %i, VISIBLE CHUNKS %i, CHUNKS TO GEN %i]\n", time.Get_FPS(), time.Get_Frame_Time(), time.Get_Update_Time(), time.Get_Render_Time(), m_drawn_voxels, m_chunks.Size(), m_visible_chunks.Size(), m_dirty_chunks.Size());	
		s_emit_fps_time = 0.0f;
	}
// DEBUG =======================================================================================
}

void ChunkManager::Update_Chunk_Lists()
{
	Update_Unload_List();
	Update_Load_List();

	// Make sure to update visible list after load list as 
	// load may produce extra chunks which will need to be visible.
	Update_Visible_List();
}

void ChunkManager::Update_Visible_List()
{
	m_visible_chunks.Clear();

	// Look for all chunks in our draw distance.
	for (int x = m_last_camera_chunk_position.X - m_config.draw_distance.X; x <= m_last_camera_chunk_position.X + m_config.draw_distance.X; x++)
	{
		for (int y = m_last_camera_chunk_position.Y - m_config.draw_distance.Y; y <= m_last_camera_chunk_position.Y + m_config.draw_distance.Y; y++)
		{
			for (int z = m_last_camera_chunk_position.Z - m_config.draw_distance.Z; z <= m_last_camera_chunk_position.Z + m_config.draw_distance.Z; z++)
			{
				Chunk* c = Get_Chunk(IntVector3(x, y, z));
				if (c != NULL && c->Is_Empty() == false && c->Should_Render() == true)
				{
					m_visible_chunks.Add(c);
				}
			}
		}
	}

	// Sort chunks from closest to furthest (reduces overdraw).
	// TODO: We aren't fillrate limited at the moment, so is this really a benefit?
	//m_visible_chunks.Sort(Visible_List_Sort_Comparer, false, &m_last_camera_chunk_position);
}

int ChunkManager::Visible_List_Sort_Comparer(Chunk* a, Chunk* b, void* extra)
{
	IntVector3 camera_position = *reinterpret_cast<IntVector3*>(extra);

	float distance_a = (camera_position - a->Get_Position()).Length_Squared();
	float distance_b = (camera_position - b->Get_Position()).Length_Squared();

	int sgn = FastSign(distance_b - distance_a);

	return sgn;
}

int ChunkManager::Chunk_List_Sort_Comparer(IntVector3 a, IntVector3 b, void* extra)
{
	IntVector3 camera_position = *reinterpret_cast<IntVector3*>(extra);

	float distance_a = (camera_position - a).Length_Squared();
	float distance_b = (camera_position - b).Length_Squared();

	int sgn = FastSign(distance_b - distance_a);

	return sgn;
}

void ChunkManager::Regenerate_Dirty_Chunks()
{
	Renderer* renderer = Renderer::Get();
	TaskManager* task_manager = TaskManager::Get();

	// Sort the dirty chunk queue.
	if (m_dirty_chunks_sorted == false)
	{
		m_dirty_chunks.Sort(Visible_List_Sort_Comparer, false, &m_last_camera_chunk_position);
		m_dirty_chunks_sorted = true;
	}

//	printf("Chunks:%i Tasks:%i Load:%i Tasks:%i\n", m_dirty_chunks.Size(), m_regenerate_mesh_tasks.Size(), m_chunk_load_list.Size(), m_load_tasks.Size());

	// Get regenerating chunks.
	while (m_regenerate_mesh_tasks.Size() < m_config.chunk_max_regenerate_tasks &&
		   m_dirty_chunks.Size() > 0)
	{
		Chunk* chunk = NULL;

		// Only regenerate chunks whos neighbours are loaded.
		for (LinkedList<Chunk*>::Iterator iter = m_dirty_chunks.Begin(); 
			iter != m_dirty_chunks.End();
			iter++)
		{
			Chunk* tmp = *iter;
			if (tmp->Are_Neighbours_Loaded())
			{
				m_dirty_chunks.Remove(iter);
				chunk = tmp;
				break;
			}
		}

		if (chunk == NULL)
		{
			break;
		}

		// Mark chunk as regenerating.
		chunk->Mark_Dirty(false);	
		chunk->Set_Regenerating(true);

		// Create regeneration task.
		ChunkRegenerateMeshTask* task = new ChunkRegenerateMeshTask(this, chunk, m_config);
		
		TaskID id = task_manager->Add_Task(task);
		task_manager->Queue_Task(id);

		m_regenerate_mesh_tasks.Add(task);
	}
}

void ChunkManager::Queue_Dirty_Chunk(Chunk* chunk)
{
	m_dirty_chunks.Add(chunk);
	m_dirty_chunks_sorted = false;
}

FixedMemoryPool<Voxel>& ChunkManager::Get_Voxel_Memory_Pool()
{
	return m_voxel_memory_pool;
}

FixedMemoryPool<Chunk>& ChunkManager::Get_Chunk_Memory_Pool()
{
	return m_chunk_memory_pool;
}

void ChunkManager::Add_Chunk(IntVector3 position, Chunk* chunk)
{
	chunk->m_chunklist_node = m_chunk_list.Add(chunk);
	m_chunks.Set(position.X, position.Y, position.Z, chunk);

	// Should add to visibility list?
	int distance_x = abs(position.X - m_last_camera_chunk_position.X);
	int distance_y = abs(position.Y - m_last_camera_chunk_position.Y);
	int distance_z = abs(position.Z - m_last_camera_chunk_position.Z);

	if (distance_x <= m_config.draw_distance.X &&
		distance_y <= m_config.draw_distance.Y &&
		distance_z <= m_config.draw_distance.Z &&
		chunk->Is_Empty() == false )
	{
		m_visible_chunks.Add(chunk);
	}

	// Make this chunk and all it's neighbours relink their neighbour array.
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{			
			for (int z = -1; z <= 1; z++)
			{
				Chunk* c = Get_Chunk(IntVector3(position.X + x, position.Y + y, position.Z + z));
				if (c != NULL)
				{
					c->Relink_Neighbours();
				}
			}
		}
	}
}

void ChunkManager::Remove_Chunk(IntVector3 position)
{
	Chunk* chunk = m_chunks.Get(position.X, position.Y, position.Z);
	DBG_ASSERT(chunk != NULL);

	if (chunk->Is_Dirty())
	{
		m_dirty_chunks.Remove(m_dirty_chunks.Find(chunk));
	}

	const LinkedList<Chunk*>::Node* node = m_visible_chunks.Find(chunk);
	if (node != NULL)
	{
		m_visible_chunks.Remove(node);
	}

	m_chunk_list.Remove(chunk->m_chunklist_node);
	m_chunks.Remove(position.X, position.Y, position.Z);
	
	// Make this chunk and all it's neighbours relink their neighbour array.
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{			
			for (int z = -1; z <= 1; z++)
			{
				Chunk* c = Get_Chunk(IntVector3(position.X + x, position.Y + y, position.Z + z));
				if (c != NULL)
				{
					c->Relink_Neighbours();
				}
			}
		}
	}
}

LinkedList<Chunk*>& ChunkManager::Get_Chunks()
{
	return m_chunk_list;
}

Chunk* ChunkManager::Get_Chunk(IntVector3 position)
{
	Chunk* chunk = m_chunks.Get(position.X, position.Y, position.Z);
	return chunk;
}

Chunk* ChunkManager::Get_Chunk(int hash)
{
	return m_chunks.Get(hash);
}

Chunk* ChunkManager::Get_Chunk_By_Position(Vector3 position)
{
	IntVector3 pos = Get_Chunk_Position(position);
	return m_chunks.Get(pos.X, pos.Y, pos.Z);
}

IntVector3 ChunkManager::Get_Chunk_Position(Vector3 position)
{
	float chunk_width  = m_config.chunk_size.X * m_config.voxel_size.X;
	float chunk_height = m_config.chunk_size.Y * m_config.voxel_size.Y;
	float chunk_depth  = m_config.chunk_size.Z * m_config.voxel_size.Z;
	
	return IntVector3(floor(position.X / chunk_width),
					  floor(position.Y / chunk_height),
				 	  floor( position.Z / chunk_depth));
}

IntVector3 ChunkManager::Get_Last_Camera_Chunk_Position()	
{
	return m_last_camera_chunk_position;
}

AABB ChunkManager::Calculate_Chunk_AABB(IntVector3 position)
{
	return AABB(position.X * (m_config.chunk_size.X * m_config.voxel_size.X),
				position.Y * (m_config.chunk_size.Y * m_config.voxel_size.Y),
				position.Z * (m_config.chunk_size.Z * m_config.voxel_size.Z),
				(m_config.chunk_size.X * m_config.voxel_size.X),
				(m_config.chunk_size.Y * m_config.voxel_size.Y),
				(m_config.chunk_size.Z * m_config.voxel_size.Z)
				);
}

WorldFile* ChunkManager::Get_World_File()
{
	return m_world_file;
}

RegionFile* ChunkManager::Get_Region_File(IntVector3 position)
{
	MutexLock lock(m_region_access_mutex);

	RegionFile* region = m_region_files_array.Get(position.X, position.Y, position.Z);
	if (region == NULL)
	{
		region = new RegionFile(position, this, m_config);
		m_region_files.Add(region);
		m_region_files_array.Set(position.X, position.Y, position.Z, region);
		
		bool ret = region->Open();
		DBG_ASSERT(ret == true);
	}

	return region;
}