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

#include "Engine\Platform\Platform.h"

#include "Game\Runner\Game.h"

#include "Engine\Input\Input.h"

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
	Camera* camera		= Game::Get()->Get_Camera(CameraID::Game);
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
	
	/*
	// ================================================================================
	// Raycasting test 
	Game*			game			= Game::Get();
	ChunkManager*	chunk_manager	= game->Get_Chunk_Manager();
	Input*			input			= Input::Get();
	MouseState*		mouse_state		= input->Get_Mouse_State();
	Point			mouse_position	= mouse_state->Get_Position();
	Vector3			voxel_size		= m_config.voxel_size;

	Camera* camera		= game->Get_Camera(CameraID::Editor_Main);
	Vector3 near		= camera->Unproject(Vector3(mouse_position.X, mouse_position.Y, 0.0f));
	Vector3	far			= camera->Unproject(Vector3(mouse_position.X, mouse_position.Y, 1.0f));
	//Vector3 start		= camera->Unproject(Vector3(mouse_position.X, mouse_position.Y, 0.0f));
	Vector3 direction	= (far - near).Normalize();
	//Vector3 direction	= (far - start).Normalize();
	Vector3 end			= near + (direction * 10.f);
	
//	near  = Vector3(1.234f, 0.123f, 2.234f);
//	end   = Vector3(9.8f, 6.2f, 9.3f);
	
	std::vector<ChunkRaycastResult> results;
	
	if (Raycast(near, end, results, 1) > 0)
	{
		Renderer*				renderer	= Renderer::Get();
		RenderPipeline_Shader*	shader		= pipeline->Get_Shader_From_Name("solid_quad");
	
		pipeline->Apply_Shader(time, shader); 
		pipeline->Update_Shader_Uniforms();
		shader->Shader_Program->Bind_Vector("g_color", Color::Red.To_Vector4());	
	
		renderer->Set_World_Matrix(Matrix4::Identity());
		pipeline->Update_Shader_Uniforms();
		renderer->Draw_Line(near.X, near.Y, near.Z, end.X, end.Y, end.Z, 8.0f);
		
		for (auto iter = results.begin(); iter != results.end(); iter++)
		{
			ChunkRaycastResult& result = *iter;
			renderer->Set_World_Matrix(Matrix4::Translate(Vector3(result.AbsoluteVoxel.X, result.AbsoluteVoxel.Y, result.AbsoluteVoxel.Z) * voxel_size));
			//renderer->Set_World_Matrix(Matrix4::Translate(Vector3(result.Test.X, result.Test.Y, result.Test.Z)));
			pipeline->Apply_Shader(time, shader); 
			pipeline->Update_Shader_Uniforms();
			shader->Shader_Program->Bind_Vector("g_color", Color::Green.To_Vector4());	
		
		renderer->Draw_Cube(m_config.voxel_size.X, m_config.voxel_size.Y, m_config.voxel_size.Z);
		}

		renderer->Set_World_Matrix(Matrix4::Identity());
		pipeline->Update_Shader_Uniforms();
	}
	// ================================================================================
	*/

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

IntVector3 ChunkManager::Get_Absolute_Voxel_Position(Vector3 position)
{
	int x = floor(position.X / m_config.voxel_size.X);
	int y = floor(position.Y / m_config.voxel_size.Y);
	int z = floor(position.Z / m_config.voxel_size.Z);

	return IntVector3(x, y, z); 
}

void ChunkManager::Get_Chunk_By_Absolute_Voxel_Position(IntVector3 position, Chunk** chunk, IntVector3* relative_position)
{
	int x = floor(position.X / (float)m_config.chunk_size.X);
	int y = floor(position.Y / (float)m_config.chunk_size.Y);
	int z = floor(position.Z / (float)m_config.chunk_size.Z);
	
	int s_x = x * m_config.chunk_size.X;
	int s_y = y * m_config.chunk_size.Y;
	int s_z = z * m_config.chunk_size.Z;

	*chunk = Get_Chunk(IntVector3(x, y, z));

	*relative_position = IntVector3
	(
		position.X - s_x,
		position.Y - s_y,
		position.Z - s_z
	);
}

IntVector3 ChunkManager::Get_Relative_Voxel_Position(Vector3 position)
{
	float chunk_width  = m_config.chunk_size.X * m_config.voxel_size.X;
	float chunk_height = m_config.chunk_size.Y * m_config.voxel_size.Y;
	float chunk_depth  = m_config.chunk_size.Z * m_config.voxel_size.Z;
	
	IntVector3 chunk_offset = Get_Chunk_Position(position);
	Vector3 voxel_offset = Vector3
	(
		position.X - (chunk_offset.X * chunk_width),
		position.Y - (chunk_offset.Y * chunk_height),
		position.Z - (chunk_offset.Z * chunk_depth)
	);

	return IntVector3
	(
		floor(voxel_offset.X / m_config.voxel_size.X),
		floor(voxel_offset.Y / m_config.voxel_size.Y),
		floor(voxel_offset.Z / m_config.voxel_size.Z)
	);

	/*
	IntVector3 rel = Get_Absolute_Voxel_Position(position);
	IntVector3 a = rel;

	if (a.X >= 0)
		a.X = (a.X % m_config.chunk_size.X);
	else
		a.X = m_config.chunk_size.X - ((abs(a.X) % m_config.chunk_size.X) + 1);

	if (a.Y >= 0)
		a.Y = (a.Y % m_config.chunk_size.Y);
	else
		a.Y = m_config.chunk_size.Y - ((abs(a.Y) % m_config.chunk_size.Y) + 1);

	if (a.Z >= 0)
		a.Z = (a.Z % m_config.chunk_size.Z);
	else
		a.Z = m_config.chunk_size.Z - ((abs(a.Z) % m_config.chunk_size.Z) + 1);

	return a;
	*/
}

IntVector3 ChunkManager::Get_Last_Camera_Chunk_Position()	
{
	return m_last_camera_chunk_position;
}

AABB ChunkManager::Calculate_Chunk_AABB(IntVector3 position)
{
	return AABB
	(
		position.X * (m_config.chunk_size.X * m_config.voxel_size.X),
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

void ChunkManager::Change_Voxel(IntVector3 absolute_position, IntVector3 face_normal, VoxelType::Type type, Color color, bool overwrite)
{
	Chunk*	   chunk				= NULL;
	IntVector3 relative_position	= IntVector3(0, 0, 0);
	Get_Chunk_By_Absolute_Voxel_Position(absolute_position, &chunk, &relative_position);

	if (chunk != NULL)
	{
		IntVector3 voxel_position = relative_position;
		Voxel	   voxel		  = *chunk->Get_Voxel(relative_position.X, relative_position.Y, relative_position.Z);

		// If we are not allowed to overwrite, then choose then place the voxel on the face-normal instead.
		if (overwrite == false && voxel.Type != VoxelType::Empty && type != VoxelType::Empty)
		{
			voxel_position = absolute_position + face_normal;

			Get_Chunk_By_Absolute_Voxel_Position(voxel_position, &chunk, &voxel_position);
			if (chunk == NULL)
			{
				return;
			}

			voxel = *chunk->Get_Voxel(relative_position.X, relative_position.Y, relative_position.Z);
		}
		
		// Work out resulting voxel state.
		voxel.ColorIndex	= chunk->Color_To_Palette_Index(color);
		voxel.Type			= type;

		chunk->Set_Voxel(voxel_position.X, voxel_position.Y, voxel_position.Z, voxel);

		// Make sure chunk is in render-list.		
		if (!m_visible_chunks.Contains(chunk))
		{
			m_visible_chunks.Add(chunk);		
		}
	}
}

int ChunkManager::Raycast(Vector3 start, Vector3 end, std::vector<ChunkRaycastResult>& results, int max_hits, ChunkRaycastBoundries* boundries)
{
	// Get voxel information.
	float voxel_size = m_config.voxel_size.X; // Should deal with different dimensional voxels.
	float voxel_size_inverse = 1.0f / voxel_size;

	// Work out direction.
	Vector3 direction = (end - start).Normalize();

	// Work out length of ray.
	float length = (end - start).Length();
	
	// Calculate the step.
	Vector3 step = (direction * voxel_size) * 0.2f;	// 5 sample points per voxels "length", accurate enough >_>, we need to do a proper line-tracing algorithm for this.
	float step_length = step.Length();

	// Resize vector to a rough estimate of how many hits at max we could have at max.	
	if (max_hits > 0)
		results.reserve(Min(ceilf(length / voxel_size), max_hits));
	else
		results.reserve(ceilf(length / voxel_size));

	// Voxel positioning.
	IntVector3	chunk_size					= m_config.chunk_size;
	float		chunk_width					= m_config.chunk_size.X * m_config.voxel_size.X;
	float		chunk_height				= m_config.chunk_size.Y * m_config.voxel_size.Y;
	float		chunk_depth					= m_config.chunk_size.Z * m_config.voxel_size.Z;
	float		inverse_chunk_width			= 1.0f / chunk_width;
	float		inverse_chunk_height		= 1.0f / chunk_height;
	float		inverse_chunk_depth			= 1.0f / chunk_depth;
	float		inverse_voxels_width		= 1.0f / m_config.voxel_size.X;
	float		inverse_voxels_height		= 1.0f / m_config.voxel_size.Y;
	float		inverse_voxels_depth		= 1.0f / m_config.voxel_size.Z;

	// Tracking variables.
	Vector3		offset				= start - step;
	int			hits				= 0;
	Chunk*		chunk				= NULL;
	IntVector3	last_chunk_position;
	IntVector3	last_voxel_position;
	bool		has_last_voxel_position = false;

	// Boundry tracking.
	//bool				found_boundry_result = false;
//	ChunkRaycastResult	boundry_result;

	// Increment over the length of the ray.
	while (length > 0)
	{
		// Get the current chunk (inlined here with inverse sizes for a bit of extra speeedz).
		IntVector3 chunk_position = IntVector3
		(
			floor(offset.X * inverse_chunk_width),
			floor(offset.Y * inverse_chunk_height),
			floor(offset.Z * inverse_chunk_depth)
		);
		if (chunk == NULL || last_chunk_position != chunk_position)
		{
			chunk = Get_Chunk(chunk_position);
		}

		// Calculate what relative voxel we are.
		Vector3 voxel_offset = Vector3
		(
			offset.X - (chunk_position.X * chunk_width),
			offset.Y - (chunk_position.Y * chunk_height),
			offset.Z - (chunk_position.Z * chunk_depth)
		);

		IntVector3 rel_voxel_position = IntVector3
		(
			floor(voxel_offset.X * inverse_voxels_width),
			floor(voxel_offset.Y * inverse_voxels_height),
			floor(voxel_offset.Z * inverse_voxels_depth)
		);
		
		IntVector3 abs_voxel_position = IntVector3
		(
			floor(offset.X * inverse_voxels_width),
			floor(offset.Y * inverse_voxels_height),
			floor(offset.Z * inverse_voxels_depth)
		);
		// Is this a hit?
		if (chunk != NULL && has_last_voxel_position == true)
		{
			ChunkRaycastResult result;
			result.Chunk			= chunk;
			result.Normal			= last_voxel_position - abs_voxel_position;
			result.AbsoluteVoxel	= (chunk_position * m_config.chunk_size) + rel_voxel_position;
			result.RelativeVoxel	= rel_voxel_position;

			Voxel* voxel = chunk->Get_Voxel(result.RelativeVoxel.X, result.RelativeVoxel.Y, result.RelativeVoxel.Z);
			if (voxel != NULL)
			{
				bool store = false;

				if (voxel->Type != VoxelType::Empty)
				{
					store = true;
				}
				else
				{
					if (boundries != NULL)
					{
						if ((boundries->Use_XBoundry == true && result.AbsoluteVoxel.X == boundries->XBoundry) ||
							(boundries->Use_YBoundry == true && result.AbsoluteVoxel.Y == boundries->YBoundry) ||
							(boundries->Use_ZBoundry == true && result.AbsoluteVoxel.Z == boundries->ZBoundry))
						{
							store = true;
						}
					}
				}

				if (store == true)
				{
					// Store result.
					results.push_back(result);

					// Maximum hits?
					hits++;
					if (max_hits > 0 && hits >= max_hits)
					{
						break;
					}
				}
			}
		}

		// Step through ray.
		while (true)
		{
			Vector3 old = offset;
			offset = offset + step;
			length -= step_length;

			// Have we got to a new voxel yet.
			if (floor(offset.X * voxel_size_inverse) != floor(old.X * voxel_size_inverse) ||
				floor(offset.Y * voxel_size_inverse) != floor(old.Y * voxel_size_inverse) ||
				floor(offset.Z * voxel_size_inverse) != floor(old.Z * voxel_size_inverse))
			{
				break;
			}
		}

		last_voxel_position = abs_voxel_position;
		has_last_voxel_position = true;
	}

	// No hits? Did we find a hit on the boundry?
	//if (hits == 0 && found_boundry_result == true)
	//{
	//	results.push_back(boundry_result);
	//	hits++;
	//}

	// Return the number of voxels we found!
	return hits;
}


/*
int ChunkManager::Raycast(Vector3 start, Vector3 end, std::vector<ChunkRaycastResult>& results, int max_hits)
{
	// Reset the results array.
	results.clear();

	// Get voxel information.
	Vector3 voxel_size = m_config.voxel_size; // Should deal with different dimensional voxels.

	// Convert to 'cube' coordinates.
	float start_x	= (start.X / voxel_size.X);
	float start_y	= (start.Y / voxel_size.Y);
	float start_z	= (start.Z / voxel_size.Z);
	float end_x		= (end.X / voxel_size.X);
	float end_y		= (end.Y / voxel_size.Y);
	float end_z		= (end.Z / voxel_size.Z);
	float radius	= (end - start).Length();

	// Work out cube containing origin.
	float x = floor(start_x); 
	float y = floor(start_y); 
	float z = floor(start_z); 

	// Work out direction.
	Vector3 direction = (end - start).Normalize();
	float dx = direction.X;
	float dy = direction.Y;
	float dz = direction.Z;

	// 0 length direction D:
	if (dx == 0 && dy == 0 && dz == 0)
	{
		return 0;
	}

	// Step increment.
	float stepX = (dx);
	float stepY = (dy);
	float stepZ = (dz);

	// Max values.
	float tMaxX = RaycastIntBound(start_x, dx);
	float tMaxY = RaycastIntBound(start_y, dy);
	float tMaxZ = RaycastIntBound(start_z, dz);

	// Delta change.
	float tDeltaX = dx == 0 ? 0 : stepX / dx;
	float tDeltaY = dy == 0 ? 0 : stepY / dy;
	float tDeltaZ = dz == 0 ? 0 : stepZ / dz;

	// Face delta.
	Vector3 face(0, 0, 0);

	// Rescale units to 1 cube-edge units.
	radius /= sqrtf(dx * dx + dy * dy + dz * dz) * voxel_size.X;
	radius = floor(radius);

	// Get casting.
	while (true)
	{
		// Store cube!
		IntVector3	chunk_size				= m_config.chunk_size;
		Vector3		voxel_position			= (Vector3(x, y, z) * voxel_size);
		IntVector3	chunk_position			= Get_Chunk_Position(voxel_position);
		//IntVector3  abs_voxel_position	= Get_Absolute_Voxel_Position(voxel_position);
		IntVector3  rel_voxel_position		= Get_Relative_Voxel_Position(voxel_position);

		// Is this a hit?
		Chunk* chunk = Get_Chunk(chunk_position);
		if (chunk != NULL)
		{
			ChunkRaycastResult result;
			result.Chunk			= chunk;
			result.Normal			= face;
			result.AbsoluteVoxel	= (chunk_position * m_config.chunk_size) + rel_voxel_position;//abs_voxel_position;
			result.RelativeVoxel	= rel_voxel_position;

			Voxel* voxel = chunk->Get_Voxel(result.RelativeVoxel.X, result.RelativeVoxel.Y, result.RelativeVoxel.Z);
			if (voxel != NULL && voxel->Type != VoxelType::Empty)
			{
				results.push_back(result);
			}
		}

		// Maximum hits?
		if (max_hits > 0)
		{
			if (results.size() >= max_hits)
			{
				break;
			}
		}

		// Step onwards.
		if (tMaxX < tMaxY)
		{
			if (tMaxX < tMaxZ)
			{
				if (tMaxX > radius)
					break;

				x += stepX;
				tMaxX += tDeltaX;
				face = Vector3(-stepX, 0, 0);
			}
			else
			{
				if (tMaxZ > radius)
					break;

				z += stepZ;
				tMaxZ += tDeltaZ;
				face = Vector3(0, 0, -stepZ);
			}
		}
		else
		{
			if (tMaxY < tMaxZ)
			{
				if (tMaxY > radius)
					break;

				y += stepY;
				tMaxY += tDeltaY;
				face = Vector3(0, -stepY, 0);
			}
			else
			{
				if (tMaxZ > radius)
					break;

				z += stepZ;
				tMaxZ += tDeltaZ;
				face = Vector3(0, 0, -stepZ);
			}
		}
	}

	// Return the number of chunks we found!
	return results.size();
}
*/



/*
int ChunkManager::Raycast(Vector3 start, Vector3 end, std::vector<ChunkRaycastResult>& results, int max_hits)
{
	// Reset the results array.
	results.clear();

	// Get voxel information.
	Vector3 voxel_size = m_config.voxel_size; // Should deal with different dimensional voxels.

	// Offset by half a cube.
	//start.X -= (voxel_size.X * 0.5f);
	//start.Y -= (voxel_size.Y * 0.5f);
	//start.Z -= (voxel_size.Z * 0.5f);
	//end.X -= (voxel_size.X * 0.5f);
	//end.Y -= (voxel_size.Y * 0.5f);
	//end.Z -= (voxel_size.Z * 0.5f);

	// Convert to 'cube' coordinates.
	float start_x	= RoundToNonZero(start.X / voxel_size.X);
	float start_y	= RoundToNonZero(start.Y / voxel_size.Y);
	float start_z	= RoundToNonZero(start.Z / voxel_size.Z);
	float end_x		= RoundToNonZero(end.X / voxel_size.X);
	float end_y		= RoundToNonZero(end.Y / voxel_size.Y);
	float end_z		= RoundToNonZero(end.Z / voxel_size.Z);
	float radius	= (end - start).Length();

	// Work out cube containing origin.
	float x = RoundToNonZero(start_x); 
	float y = RoundToNonZero(start_y); 
	float z = RoundToNonZero(start_z); 

	// Work out direction.
	Vector3 direction = (end - start).Normalize();
	float dx = direction.X;
	float dy = direction.Y;
	float dz = direction.Z;

	// 0 length direction D:
	if (dx == 0 && dy == 0 && dz == 0)
	{
		return 0;
	}

	// Step increment.
	float stepX = (dx);
	float stepY = (dy);
	float stepZ = (dz);

	// Max values.
	int tMaxX = RaycastIntBound(start_x, dx);
	int tMaxY = RaycastIntBound(start_y, dy);
	int tMaxZ = RaycastIntBound(start_z, dz);

	// Delta change.
	float tDeltaX = dx == 0 ? 0 : stepX / dx;
	float tDeltaY = dy == 0 ? 0 : stepY / dy;
	float tDeltaZ = dz == 0 ? 0 : stepZ / dz;

	// Face delta.
	Vector3 face(0, 0, 0);

	// Rescale units to 1 cube-edge units.
	radius /= sqrtf(dx * dx + dy * dy + dz * dz) * voxel_size.X;
	radius = floor(radius);

	//IntVector3 last_abs_position;
	//IntVector3 last_rel_position;
	//IntVector3 last_chunk_position;

	// Get casting.
	while (true)
	{
		// Store cube!
		IntVector3	chunk_size				= m_config.chunk_size;
		Vector3		voxel_position			= (Vector3(x, y, z) * voxel_size);
		IntVector3	chunk_position			= Get_Chunk_Position(voxel_position);
		//IntVector3  abs_voxel_position	= Get_Absolute_Voxel_Position(voxel_position);
		IntVector3  rel_voxel_position		= Get_Relative_Voxel_Position(voxel_position);

		// Is this a hit?
		Chunk* chunk = Get_Chunk(chunk_position);
		if (chunk != NULL)
		{
			ChunkRaycastResult result;
			result.Chunk			= chunk;
			result.Normal			= face;
			result.AbsoluteVoxel	= (chunk_position * m_config.chunk_size) + rel_voxel_position;//abs_voxel_position;
			result.RelativeVoxel	= rel_voxel_position;

			Voxel* voxel = chunk->Get_Voxel(result.RelativeVoxel.X, result.RelativeVoxel.Y, result.RelativeVoxel.Z);
			if (voxel != NULL)// && voxel->Type != VoxelType::Empty)
			{
				results.push_back(result);
			}
		}

		// Maximum hits?
		if (max_hits > 0)
		{
			if (results.size() >= max_hits)
			{
				break;
			}
		}

		// Step onwards.
		if (tMaxX < tMaxY)
		{
			if (tMaxX < tMaxZ)
			{
				if (tMaxX >= radius)
					break;

				x += stepX;
				tMaxX += tDeltaX;
				face = Vector3(-stepX, 0, 0);
			}
			else
			{
				if (tMaxZ >= radius)
					break;

				z += stepZ;
				tMaxZ += tDeltaZ;
				face = Vector3(0, 0, -stepZ);
			}
		}
		else
		{
			if (tMaxY <= tMaxZ)
			{
				if (tMaxY > radius)
					break;

				y += stepY;
				tMaxY += tDeltaY;
				face = Vector3(0, -stepY, 0);
			}
			else
			{
				if (tMaxZ >= radius)
					break;

				z += stepZ;
				tMaxZ += tDeltaZ;
				face = Vector3(0, 0, -stepZ);
			}
		}
	}

	// Return the number of chunks we found!
	return results.size();
}
*/

/*
int ChunkManager::Raycast(Vector3 start, Vector3 end, std::vector<ChunkRaycastResult>& results, int max_hits)
{
	// Reset the results array.
	results.clear();

	// Get voxel information.
	Vector3 voxel_size = m_config.voxel_size; // Should deal with different dimensional voxels.

	// Offset by half a cube.
	//start.X += (voxel_size.X * 0.5f);
//	start.Y += (voxel_size.Y * 0.5f);
	//start.Z += (voxel_size.Z * 0.5f);
//	end.X += (voxel_size.X * 0.5f);
//	end.Y += (voxel_size.Y * 0.5f);
//	end.Z += (voxel_size.Z * 0.5f);

	// Convert to 'cube' coordinates.
	float start_x	= (start.X / voxel_size.X);
	float start_y	= (start.Y / voxel_size.Y);
	float start_z	= (start.Z / voxel_size.Z);
	float end_x		= (end.X / voxel_size.X);
	float end_y		= (end.Y / voxel_size.Y);
	float end_z		= (end.Z / voxel_size.Z);
	float radius	= (end - start).Length();

	// Work out cube containing origin.
	float x = floor(start_x); 
	float y = floor(start_y); 
	float z = floor(start_z); 

	// Work out direction.
	Vector3 direction = (end - start).Normalize();
	float dx = direction.X;
	float dy = direction.Y;
	float dz = direction.Z;

	// 0 length direction D:
	if (dx == 0 && dy == 0 && dz == 0)
	{
		return 0;
	}

	// Step increment.
	float stepX = (dx);
	float stepY = (dy);
	float stepZ = (dz);

	// Max values.
	int tMaxX = RaycastIntBound(start_x, dx);
	int tMaxY = RaycastIntBound(start_y, dy);
	int tMaxZ = RaycastIntBound(start_z, dz);

	// Delta change.
	float tDeltaX = dx == 0 ? 0 : stepX / dx;
	float tDeltaY = dy == 0 ? 0 : stepY / dy;
	float tDeltaZ = dz == 0 ? 0 : stepZ / dz;

	// Face delta.
	Vector3 face(0, 0, 0);

	// Rescale units to 1 cube-edge units.
	radius /= sqrtf(dx * dx + dy * dy + dz * dz) * voxel_size.X;

	//IntVector3 last_abs_position;
	//IntVector3 last_rel_position;
	//IntVector3 last_chunk_position;

	// Get casting.
	while (true)
	{
		// Store cube!
		IntVector3	chunk_size			= m_config.chunk_size;
		Vector3		voxel_position		= (Vector3(x, y, z) * voxel_size) + (voxel_size * 0.5f);
		IntVector3	chunk_position		= Get_Chunk_Position(voxel_position);
		//IntVector3  abs_voxel_position	= Get_Absolute_Voxel_Position(voxel_position);
		IntVector3  rel_voxel_position	= Get_Relative_Voxel_Position(voxel_position);

		// Is this a hit?
		Chunk* chunk = Get_Chunk(chunk_position);
		if (chunk != NULL)
		{
			ChunkRaycastResult result;
			result.Chunk			= chunk;
			result.Normal			= face;
			result.AbsoluteVoxel	= (chunk_position * m_config.chunk_size) + rel_voxel_position;//abs_voxel_position;
			result.RelativeVoxel	= rel_voxel_position;

			Voxel* voxel = chunk->Get_Voxel(result.RelativeVoxel.X, result.RelativeVoxel.Y, result.RelativeVoxel.Z);
			if (voxel != NULL && voxel->Type != VoxelType::Empty)
			{
				results.push_back(result);
			}
		}

		// Maximum hits?
		if (max_hits > 0)
		{
			if (results.size() >= max_hits)
			{
				break;
			}
		}

		// Step onwards.
		if (tMaxX < tMaxY)
		{
			if (tMaxX < tMaxZ)
			{
				if (tMaxX > radius)
					break;

				x += stepX;
				tMaxX += tDeltaX;
				face = Vector3(-stepX, 0, 0);
			}
			else
			{
				if (tMaxZ > radius)
					break;

				z += stepZ;
				tMaxZ += tDeltaZ;
				face = Vector3(0, 0, -stepZ);
			}
		}
		else
		{
			if (tMaxY < tMaxZ)
			{
				if (tMaxY > radius)
					break;

				y += stepY;
				tMaxY += tDeltaY;
				face = Vector3(0, -stepY, 0);
			}
			else
			{
				if (tMaxZ > radius)
					break;

				z += stepZ;
				tMaxZ += tDeltaZ;
				face = Vector3(0, 0, -stepZ);
			}
		}
	}

	// Return the number of chunks we found!
	return results.size();
}
*/

int ChunkManager::RaycastIntBound(float s, float ds)
{
	if (ds == 0)
	{
		return 9999999.0f;
	}
	else
	{
		return (ds > 0 ? ceilf(s) - s : s - floorf(s)) / abs(ds);
	}

/*	if (ds == 0)
	{
		return 9999999.0f;
	}
	if (ds < 0)
	{
		return RaycastIntBound(-s, -ds);
	}
	else
	{
		s = RaycastMod(s, 1);
		return (1.0f - s) / ds;
	}
*/
}

int ChunkManager::RaycastMod(int value, int modulus)
{
	return (value % modulus + modulus) % modulus;
}