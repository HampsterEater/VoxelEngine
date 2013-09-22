// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Voxels\ChunkManager.h"
#include "Engine\Scene\Camera.h"
#include "Generic\Math\Math.h"
#include "Generic\Types\LinkedList.h"
#include "Generic\Math\Random.h"
#include "Engine\Platform\Platform.h"
#include "Engine\Engine\GameEngine.h"
#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Textures\TextureAtlas.h"
#include "Engine\Renderer\RenderPipeline.h"

#include "Engine\Renderer\Shaders\Shader.h"
#include "Engine\Renderer\Shaders\ShaderFactory.h"
#include "Engine\Renderer\Shaders\ShaderProgram.h"

#include <algorithm>

ChunkManager::ChunkManager(const ChunkManagerConfig& config)
	: m_config(config)
	, m_last_camera_chunk_position(-9999, -9999, -9999)
	, m_last_camera_position(-9999, -9999, -9999)
	, m_chunk_loader(this, config)					// TODO: Not going to be deterministically destructed, will mess with destructor of ChunkManager, fix.
	, m_chunk_unloader(this, config)
	, m_max_chunks((m_config.unload_distance.X * 2) * 
				   (m_config.unload_distance.Y * 2) * 
				   (m_config.unload_distance.Z * 2))
	, m_voxels_per_chunk((m_config.chunk_size.X) * 
						 (m_config.chunk_size.Y) * 
						 (m_config.chunk_size.Z))
	, m_chunk_memory_pool(m_max_chunks * config.chunk_memory_pool_buffer)
	, m_voxel_memory_pool(m_max_chunks * config.voxel_memory_pool_buffer, sizeof(Voxel) * m_voxels_per_chunk)
	, m_world_file(this, config)
{
	m_voxel_face_atlas_texture = TextureFactory::Load("Data/Textures/Chunks/Voxel_Face_Atlas.png", (TextureFlags::Type)0);
	m_voxel_face_atlas = new TextureAtlas(m_voxel_face_atlas_texture, 16, 16);
	m_voxel_face_atlas_material = new Material(m_voxel_face_atlas_texture, 0.0f, Vector3(0, 0, 0));

	// Access mutexes!
	m_region_access_mutex = Mutex::Create();
	DBG_ASSERT(m_region_access_mutex != NULL);

	// Open the world file!
	bool ret = m_world_file.Open();
	DBG_ASSERT(ret == true);
}

ChunkManager::~ChunkManager()
{
	m_world_file.Close();

	for (LinkedList<RegionFile*>::Iterator iter = m_region_files.Begin(); iter != m_region_files.End(); iter++)
	{
		RegionFile* region = *iter;
		region->Close();

		SAFE_DELETE(region);
	}
	m_region_files.Clear();

	SAFE_DELETE(m_region_access_mutex);

	SAFE_DELETE(m_voxel_face_atlas);
	SAFE_DELETE(m_voxel_face_atlas_texture);	
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

		printf("Moved to chunk: %i,%i,%i\n", chunk_position.X, chunk_position.Y, chunk_position.Z);

		Update_Chunk_Lists();
	}

	// Remove newly unloaded chunks.
	int counter = 0;		
	if (m_chunk_unloader.Get_Mutex()->Try_Lock())
	{
		while ((counter++) < m_config.chunk_unloads_per_frame)
		{
			Chunk* chunk = m_chunk_unloader.Consume_Chunk();
			if (chunk == NULL)
			{
				break;
			}
			
			Remove_Chunk(chunk->Get_Position());
			m_chunk_memory_pool.Release(chunk);		
		}

		m_chunk_unloader.Get_Mutex()->Unlock();
	}

	// Insert newly loaded chunks.
	if (m_chunk_loader.Get_Mutex()->Try_Lock())
	{
		while (true)
		{
			Chunk* chunk = m_chunk_loader.Consume_Chunk();
			if (chunk == NULL)
			{
				break;
			}

			Add_Chunk(chunk->Get_Position(), chunk);
			chunk->Mark_Dirty(true);
		}

		m_chunk_loader.Get_Mutex()->Unlock();
	}

	// Regenerate dirty chunk meshes.
	Regenerate_Dirty_Chunks();
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
	if (s_emit_fps_time > 60.0f || time.Get_Frame_Time() > 5.0f)
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
	for (int x = m_last_camera_chunk_position.X - m_config.draw_distance.X; x < m_last_camera_chunk_position.X + m_config.draw_distance.X; x++)
	{
		for (int y = m_last_camera_chunk_position.Y - m_config.draw_distance.Y; y < m_last_camera_chunk_position.Y + m_config.draw_distance.Y; y++)
		{
			for (int z = m_last_camera_chunk_position.Z - m_config.draw_distance.Z; z < m_last_camera_chunk_position.Z + m_config.draw_distance.Z; z++)
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

void ChunkManager::Update_Unload_List()
{
	m_chunk_unloader.Refresh();
}

void ChunkManager::Update_Load_List()
{
	m_chunk_loader.Refresh();
}

void ChunkManager::Regenerate_Dirty_Chunks()
{
	Renderer* renderer = Renderer::Get();
	RenderPipeline* pipeline = GameEngine::Get()->Get_RenderPipeline();
	Frustum frustum	= pipeline->Get_Active_Camera()->Get_Frustum();

	// Keep loading while we have time available.
	for (int i = 0; i < m_config.chunk_regenerations_per_frame; i++)
	{
		Chunk* closest_chunk = NULL;
		const LinkedList<Chunk*>::Node* closest_chunk_node = NULL;
		float  closest_distance = 0.0f;
		
		// Find closest chunk to regenerate.
		for (LinkedList<Chunk*>::Iterator iter = m_dirty_chunks.Begin(); iter != m_dirty_chunks.End(); iter++)
		{
			Chunk* chunk		= *iter;
			AABB   chunk_aabb	= chunk->Get_AABB();
			float  distance		= (chunk_aabb.Center - m_last_camera_chunk_position).Length_Squared();

			// If chunk is in frustum, then bias it so its regenerated earlier.
			//if (frustum.Intersects(chunk_aabb) != Frustum::IntersectionResult::Outside)
			//{
			//	distance /= 100.0f;
			//}

			if (closest_chunk == NULL || distance < closest_distance)
			{
				closest_chunk = chunk;
				closest_distance = distance;
				closest_chunk_node = iter.Get_Node();
			}
		}

		// Regenerate chunk.
		if (closest_chunk == NULL)
		{
			break;
		}
		else
		{
			closest_chunk->Regenerate_Mesh(renderer);
			
		//	DBG_LOG("Regenerated chunk %i,%i,%i", closest_chunk->Get_Position().X, closest_chunk->Get_Position().Y, closest_chunk->Get_Position().Z);

			m_dirty_chunks.Remove(closest_chunk_node);
		}
	}
}

void ChunkManager::Queue_Dirty_Chunk(Chunk* chunk)
{
	m_dirty_chunks.Add(chunk);
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
}

LinkedList<Chunk*>& ChunkManager::Get_Chunks()
{
	return m_chunk_list;
}

Chunk* ChunkManager::Get_Chunk(IntVector3 position)
{
	return m_chunks.Get(position.X, position.Y, position.Z);;
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
	return &m_world_file;
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