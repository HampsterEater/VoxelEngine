// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Voxels\ChunkManager.h"
#include "Game\Scene\Voxels\Chunk.h"
#include "Generic\Types\Vector3.h"
#include "Generic\Types\AABB.h"
#include "Engine\Engine\GameEngine.h"
#include "Engine\Platform\Platform.h"
#include "Engine\Renderer\Textures\TextureAtlas.h"

#include <string.h>
#include <stdlib.h>
#include <vector>

Chunk::Chunk()
{
	// This is just here so we work with embedded linked lists.
}

Chunk::Chunk(ChunkManager* manager, int x, int y, int z, int width, int height, int depth, float voxel_width, float voxel_height, float voxel_depth)
	: m_manager(manager)
	, m_x(x)
	, m_y(y)
	, m_z(z)
	, m_width(width)
	, m_height(height)
	, m_depth(depth)
	, m_mesh_id(-1)
	, m_is_dirty(false)
	, m_voxel_width(voxel_width)
	, m_voxel_height(voxel_height)
	, m_voxel_depth(voxel_depth)
	, m_mesh_voxel_count(0)
	, m_status(ChunkStatus::Initialized)
	, m_is_empty(false)
	, m_is_full(false)
	, m_is_contained(false)
	, m_has_hole_face(false)
	, m_aabb_cached(false)
{	
	int chunk_size = sizeof(Voxel) * m_width * m_height * m_depth;
		
	void* mem = manager->Get_Voxel_Memory_Pool().Allocate();
	DBG_ASSERT(mem != NULL);

	m_voxels = (Voxel*)mem;
	memset(m_voxels, 0, sizeof(Voxel) * m_width * m_height * m_depth);
}

Chunk::~Chunk()
{
	if (m_mesh_id >= 0)
	{
		Renderer::Get()->Destroy_Mesh(m_mesh_id);
		m_mesh_id = -1;
	}

	m_manager->Get_Voxel_Memory_Pool().Release(m_voxels);
}

IntVector3 Chunk::Get_Position() const
{
	return IntVector3(m_x, m_y, m_z);
}

IntVector3 Chunk::Get_Region() const
{
	return IntVector3(
		floor((float)m_x / (float)m_manager->Get_Config().region_size.X),
		floor((float)m_y / (float)m_manager->Get_Config().region_size.Y),
		floor((float)m_z / (float)m_manager->Get_Config().region_size.Z)
	);
}

bool Chunk::Should_Render() const
{
	return m_triangle_count > 0 || m_is_dirty == true;
}

AABB Chunk::Get_AABB() 
{
	if (m_aabb_cached == false)
	{
		m_aabb_cached = true;

		m_aabb = AABB(m_x * (m_width * m_voxel_width),
						m_y * (m_height * m_voxel_height),
						m_z * (m_depth * m_voxel_depth),
						(m_width * m_voxel_width),
						(m_height * m_voxel_height),
						(m_depth * m_voxel_depth));
	}

	return m_aabb;
}

Sphere Chunk::Get_Bounding_Sphere() const
{
	// TODO: Totally fucking wrong.

	float largest_axis = (m_width * m_voxel_width);

	if ((m_height * m_voxel_height) > largest_axis)
	{
		largest_axis = (m_height * m_voxel_height);
	}
	
	if ((m_depth * m_voxel_depth) > largest_axis)
	{
		largest_axis = (m_depth * m_voxel_depth);
	}

	float radius = largest_axis;

	Vector3 center = Vector3((m_x * (m_width * m_voxel_width)) + ((m_width * m_voxel_width) * 0.5f),
							 (m_y * (m_height * m_voxel_height)) + ((m_height * m_voxel_height) * 0.5f),
							 (m_z * (m_depth * m_voxel_depth)) + ((m_depth * m_voxel_depth) * 0.5f)
							 );

	return Sphere(center, radius);
}

void Chunk::Reset_Unload_Timer() 
{
	m_unload_timer = Platform::Get()->Get_Ticks();
}

float Chunk::Get_Unload_Timer() const
{
	return Platform::Get()->Get_Ticks() - m_unload_timer;
}

int Chunk::Drawn_Voxels()
{
	return m_mesh_voxel_count;
}

bool Chunk::Is_Dirty()
{
	return m_is_dirty;
}

void Chunk::Mark_Dirty(bool dirty)
{
	if (m_is_dirty == false && dirty == true)
	{
		m_manager->Queue_Dirty_Chunk(this);
	}
	m_is_dirty = dirty;
}

void Chunk::Fill(VoxelType::Type type, int ix, int iy, int iz, int width, int height, int depth)
{
	if (width <= 0 && height <= 0 && depth <= 0)
	{
		return;
	}

	for (int x = ix; x <= ix + m_width; x++)
	{
		for (int y = 0; y <= iy + m_height; y++)
		{
			for (int z = 0; z < iz + m_depth; z++)
			{
				int hash = Flatten_Index(x, y, z);

				m_voxels[hash].Type = type;
			}
		}
	}
}

void Chunk::Set(VoxelType::Type type, int x, int y, int z)
{
	int hash = Flatten_Index(x, y, z);
	m_voxels[hash].Type = type;
}

void Chunk::Recalculate_State()
{
	m_is_full = true;
	m_is_empty = true;

	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			for (int z = 0; z < m_depth; z++)
			{
				int hash = Flatten_Index(x, y, z);

				if (m_voxels[hash].Type == VoxelType::Empty)
				{
					m_is_full = false;
				}
				else
				{
					m_is_empty = false;
				}
			}
		}
	}

	// Are we contained?
	m_is_contained = false; // TODO
}

void Chunk::Regenerate_Voxel(Renderer* renderer, const Render_Voxel& voxel, int ix, int iy, int iz)
{
	TextureAtlas* m_face_atlas = m_manager->Get_Voxel_Face_Atlas();

	float half_w = m_voxel_width / 2.0f;
	float half_h = m_voxel_height / 2.0f;
	float half_d = m_voxel_depth / 2.0f;

		//renderer->Translate_World_Matrix((float)m_x * (m_width * (m_voxel_width * 1)), 
	//							     (float)m_y * (m_height * (m_voxel_height * 1)), 
	//							     (float)m_z * (m_depth * (m_voxel_depth * 1)));


	float x = (m_x * m_width * m_voxel_width) + ((ix * m_voxel_width) + (half_w / 2.0f));
	float y = (m_y * m_height * m_voxel_height) + ((iy * m_voxel_height) + (half_h / 2.0f));
	float z = (m_z * m_depth * m_voxel_depth) + ((iz * m_voxel_depth) + (half_d / 2.0f));

	Vector3 p1(x - half_w, y - half_h, z + half_d);
	Vector3 p2(x + half_w, y - half_h, z + half_d);
	Vector3 p3(x + half_w, y + half_h, z + half_d);
	Vector3 p4(x - half_w, y + half_h, z + half_d);
	Vector3 p5(x + half_w, y - half_h, z - half_d);
	Vector3 p6(x - half_w, y - half_h, z - half_d);
	Vector3 p7(x - half_w, y + half_h, z - half_d);
	Vector3 p8(x + half_w, y + half_h, z - half_d);
	
	Vector3 n1;
    int v1, v2, v3, v4, v5, v6, v7, v8;	
    float r = 1.0f, g = 1.0f, b = 1.0f, a = 1.0f;
	float uv_t = 1.0f, uv_l = 0.0f, uv_b = 0.0f, uv_r = 0.0f;

	// Front
	if (voxel.render_front_face == true)
	{
		m_face_atlas->Get_UV_For_Index(1, uv_l, uv_t, uv_r, uv_b);

		n1 = Vector3(0.0f, 0.0f, 1.0f);

		v1 = renderer->Add_Mesh_Vertex(m_mesh_id, p1, n1, r, g, b, a, uv_l, uv_b);
		v2 = renderer->Add_Mesh_Vertex(m_mesh_id, p2, n1, r, g, b, a, uv_r, uv_b);
		v3 = renderer->Add_Mesh_Vertex(m_mesh_id, p3, n1, r, g, b, a, uv_r, uv_t);
		v4 = renderer->Add_Mesh_Vertex(m_mesh_id, p4, n1, r, g, b, a, uv_l, uv_t);

		renderer->Add_Mesh_Triangle(m_mesh_id, v1, v2, v3);
		renderer->Add_Mesh_Triangle(m_mesh_id, v1, v3, v4);
	}

    // Back
	if (voxel.render_back_face == true)
	{
		m_face_atlas->Get_UV_For_Index(1, uv_l, uv_t, uv_r, uv_b);
		
		n1 = Vector3(0.0f, 0.0f, -1.0f);

		v5 = renderer->Add_Mesh_Vertex(m_mesh_id, p5, n1, r, g, b, a, uv_l, uv_b);
		v6 = renderer->Add_Mesh_Vertex(m_mesh_id, p6, n1, r, g, b, a, uv_r, uv_b);
		v7 = renderer->Add_Mesh_Vertex(m_mesh_id, p7, n1, r, g, b, a, uv_r, uv_t);
		v8 = renderer->Add_Mesh_Vertex(m_mesh_id, p8, n1, r, g, b, a, uv_l, uv_t);

		renderer->Add_Mesh_Triangle(m_mesh_id, v5, v6, v7);
		renderer->Add_Mesh_Triangle(m_mesh_id, v5, v7, v8);
	}

    // Right
	if (voxel.render_right_face == true)
	{
		m_face_atlas->Get_UV_For_Index(1, uv_l, uv_t, uv_r, uv_b);

		n1 = Vector3(1.0f, 0.0f, 0.0f);

		v2 = renderer->Add_Mesh_Vertex(m_mesh_id, p2, n1, r, g, b, a, uv_l, uv_b);
		v5 = renderer->Add_Mesh_Vertex(m_mesh_id, p5, n1, r, g, b, a, uv_r, uv_b);
		v8 = renderer->Add_Mesh_Vertex(m_mesh_id, p8, n1, r, g, b, a, uv_r, uv_t);
		v3 = renderer->Add_Mesh_Vertex(m_mesh_id, p3, n1, r, g, b, a, uv_l, uv_t);

		renderer->Add_Mesh_Triangle(m_mesh_id, v2, v5, v8);
		renderer->Add_Mesh_Triangle(m_mesh_id, v2, v8, v3);
	}

    // left
	if (voxel.render_left_face == true)
	{
		m_face_atlas->Get_UV_For_Index(1, uv_l, uv_t, uv_r, uv_b);

		n1 = Vector3(-1.0f, 0.0f, 0.0f);

		v6 = renderer->Add_Mesh_Vertex(m_mesh_id, p6, n1, r, g, b, a, uv_l, uv_b);
		v1 = renderer->Add_Mesh_Vertex(m_mesh_id, p1, n1, r, g, b, a, uv_r, uv_b);
		v4 = renderer->Add_Mesh_Vertex(m_mesh_id, p4, n1, r, g, b, a, uv_r, uv_t);
		v7 = renderer->Add_Mesh_Vertex(m_mesh_id, p7, n1, r, g, b, a, uv_l, uv_t);

		renderer->Add_Mesh_Triangle(m_mesh_id, v6, v1, v4);
		renderer->Add_Mesh_Triangle(m_mesh_id, v6, v4, v7);
	}

    // Top
	if (voxel.render_top_face == true)
	{
		m_face_atlas->Get_UV_For_Index(2, uv_l, uv_t, uv_r, uv_b);

		n1 = Vector3(0.0f, 1.0f, 0.0f);

		v4 = renderer->Add_Mesh_Vertex(m_mesh_id, p4, n1, r, g, b, a, uv_l, uv_t);
		v3 = renderer->Add_Mesh_Vertex(m_mesh_id, p3, n1, r, g, b, a, uv_l, uv_b);
		v8 = renderer->Add_Mesh_Vertex(m_mesh_id, p8, n1, r, g, b, a, uv_r, uv_b);
		v7 = renderer->Add_Mesh_Vertex(m_mesh_id, p7, n1, r, g, b, a, uv_r, uv_t);

		renderer->Add_Mesh_Triangle(m_mesh_id, v4, v3, v8);
		renderer->Add_Mesh_Triangle(m_mesh_id, v4, v8, v7);
	}

    // Bottom
	if (voxel.render_bottom_face == true)
	{
		m_face_atlas->Get_UV_For_Index(0, uv_l, uv_t, uv_r, uv_b);

		n1 = Vector3(0.0f, -1.0f, 0.0f);

		v6 = renderer->Add_Mesh_Vertex(m_mesh_id, p6, n1, r, g, b, a, uv_r, uv_t);
		v5 = renderer->Add_Mesh_Vertex(m_mesh_id, p5, n1, r, g, b, a, uv_l, uv_t);
		v2 = renderer->Add_Mesh_Vertex(m_mesh_id, p2, n1, r, g, b, a, uv_l, uv_b);
		v1 = renderer->Add_Mesh_Vertex(m_mesh_id, p1, n1, r, g, b, a, uv_r, uv_b);

		renderer->Add_Mesh_Triangle(m_mesh_id, v6, v5, v2);
		renderer->Add_Mesh_Triangle(m_mesh_id, v6, v2, v1);
	}
}

bool Chunk::Should_Render_Voxel(Render_Voxel& voxel)
{
	// Voxel dosen't contain anything?
	if (voxel.voxel->Type == VoxelType::Empty)
	{
		return false;
	}

	// Set intial settings.
	voxel.triangle_count = 0;
	voxel.vertex_count = 0;

	// Work out what faces to render.	
	Voxel* voxel_above = Get_Relative_Voxel(voxel.x, voxel.y, voxel.z, 0,  1,  0);
	Voxel* voxel_below = Get_Relative_Voxel(voxel.x, voxel.y, voxel.z, 0, -1,  0);
	Voxel* voxel_front = Get_Relative_Voxel(voxel.x, voxel.y, voxel.z, 0,  0,  1);
	Voxel* voxel_back  = Get_Relative_Voxel(voxel.x, voxel.y, voxel.z, 0,  0, -1);
	Voxel* voxel_left  = Get_Relative_Voxel(voxel.x, voxel.y, voxel.z,-1,  0,  0);
	Voxel* voxel_right = Get_Relative_Voxel(voxel.x, voxel.y, voxel.z, 1,  0,  0);

	// Are we going to have a hole for a face? :(
	if (voxel_above == NULL || voxel_below == NULL || voxel_front == NULL || voxel_back == NULL || voxel_left == NULL || voxel_right == NULL)
	{
		m_has_hole_face = true;
	}

	// Top face?
	if (voxel_above == NULL || voxel_above->Type != VoxelType::Empty)
	{
		voxel.render_top_face = false;
	}
	else
	{
		voxel.render_top_face = true;
		voxel.triangle_count  += 2;
		voxel.vertex_count    += 4;
	}

	// Bottom face?
	if (voxel_below == NULL || voxel_below->Type != VoxelType::Empty)
	{
		voxel.render_bottom_face = false;
	}
	else
	{
		voxel.render_bottom_face = true;
		voxel.triangle_count  += 2;
		voxel.vertex_count    += 4;
	}
	
	// Front face?
	if (voxel_front == NULL || voxel_front->Type != VoxelType::Empty)
	{
		voxel.render_front_face = false;
	}
	else
	{
		voxel.render_front_face = true;
		voxel.triangle_count  += 2;
		voxel.vertex_count    += 4;
	}

	// Back face?
	if (voxel_back == NULL || voxel_back->Type != VoxelType::Empty)
	{
		voxel.render_back_face = false;
	}
	else
	{
		voxel.render_back_face = true;
		voxel.triangle_count  += 2;
		voxel.vertex_count    += 4;
	}
	
	// Left face?
	if (voxel_left == NULL || voxel_left->Type != VoxelType::Empty)
	{
		voxel.render_left_face = false;
	}
	else
	{
		voxel.render_left_face = true;
		voxel.triangle_count  += 2;
		voxel.vertex_count    += 4;
	}
	
	// Right face?
	if (voxel_right == NULL || voxel_right->Type != VoxelType::Empty)
	{
		voxel.render_right_face = false;
	}
	else
	{
		voxel.render_right_face = true;
		voxel.triangle_count  += 2;
		voxel.vertex_count    += 4;
	}

	// No faces to render? Don't render then lol.
	if (voxel.triangle_count == 0)
	{
		return false;
	}

	return true;
}

void Chunk::Regenerate_Mesh(Renderer* renderer, bool as_neighbour)
{
	// Get rid of old mesh?	
	if (m_mesh_id >= 0)
	{
		renderer->Destroy_Mesh(m_mesh_id);
		m_mesh_id = -1;
	}

	// Reset some info.	
	m_has_hole_face = false;

	// Work out a list of candidate voxels for rendering.
	std::vector<Render_Voxel> render_voxels;
	int render_vertices = 0;
	int render_triangles = 0;

	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			for (int z = 0; z < m_depth; z++)
			{
				Voxel& voxel = m_voxels[Flatten_Index(x, y, z)];

				Render_Voxel render_voxel = Render_Voxel(&voxel, x, y, z);

				if (Should_Render_Voxel(render_voxel))
				{
					render_voxels.push_back(render_voxel);
					render_triangles += render_voxel.triangle_count;
					render_vertices  += render_voxel.vertex_count;
				}
			}
		}
	}

	// Regenerate new mesh!
	m_mesh_voxel_count = render_voxels.size();

	if (render_triangles > 0)
	{
		m_mesh_id = renderer->Start_Mesh(render_vertices, render_triangles);
		DBG_ASSERT(m_mesh_id >= 0);

		for (std::vector<Render_Voxel>::iterator iter = render_voxels.begin(); iter != render_voxels.end(); iter++)
		{
			Render_Voxel& render_voxel = *iter;		
			Regenerate_Voxel(renderer, render_voxel, render_voxel.x, render_voxel.y, render_voxel.z);
		}

		renderer->End_Mesh(m_mesh_id);
	}

	// Should we regenerate neighbours?
	// TODO: Only do this if this is a new chunk or we have modified edge voxels.
	if (as_neighbour == false)
	{
		Regenerate_Neighbour_Meshs(renderer);
	}

	m_triangle_count = render_triangles;
	m_is_dirty = false;
}

void Chunk::Regenerate_Neighbour_Meshs(Renderer* renderer)
{
	for (int x = -1; x <= 1; x++)
	{	
		for (int y = -1; y <= 1; y++)
		{			
			for (int z = -1; z <= 1; z++)
			{
				// Only do cardinal directions.
				if ((x != 0 && y == 0 && z == 0) ||
					(x == 0 && y != 0 && z == 0) ||
					(x == 0 && y == 0 && z != 0))
				{
					Chunk* c = m_manager->Get_Chunk(IntVector3(m_x + x, 
															   m_y + y, 
															   m_z + z));
					if (c != NULL && c->Is_Dirty() == false)
					{
						c->Regenerate_Mesh(renderer, true);
					}
				}
			}
		}
	}
}

Voxel* Chunk::Get_Voxel(int x, int y, int z)
{
	if (m_status != ChunkStatus::Loaded)
	{
		return NULL;
	}

	return &m_voxels[Flatten_Index(x, y, z)];
}

Voxel* Chunk::Get_Relative_Voxel(int voxel_x, int voxel_y, int voxel_z, 
								 int offset_x, int offset_y, int offset_z)
{
	int rel_x = voxel_x + offset_x;
	int rel_y = voxel_y + offset_y;
	int rel_z = voxel_z + offset_z;

	int chunk_x = m_x;
	int chunk_y = m_y;
	int chunk_z = m_z;

	while (rel_x < 0)
	{
		rel_x += m_width;
		chunk_x--;
	}
	while (rel_x >= m_width)
	{
		rel_x -= m_width;
		chunk_x++;
	}
	while (rel_y < 0)
	{
		rel_y += m_height;
		chunk_y--;
	}
	while (rel_y >= m_height)
	{
		rel_y -= m_height;
		chunk_y++;
	}
	while (rel_z < 0)
	{
		rel_z += m_depth;
		chunk_z--;
	}
	while (rel_z >= m_depth)
	{
		rel_z -= m_depth;
		chunk_z++;
	}

	Chunk* chunk = this;
	if (chunk_x != m_x || chunk_y != m_y || chunk_z != m_z)
	{
		chunk = m_manager->Get_Chunk(IntVector3(chunk_x, chunk_y, chunk_z));
	}

	if (chunk != NULL && chunk->Get_Status() == ChunkStatus::Loaded)
	{
		return chunk->Get_Voxel(rel_x, rel_y, rel_z);
	}

	return NULL;
}

void Chunk::Draw(const FrameTime& time, RenderPipeline* pipeline)
{
	Renderer* renderer = Renderer::Get();

	if (m_mesh_id >= 0)
	{
		renderer->Render_Mesh(m_mesh_id);
	}
}

void Chunk::Tick(const FrameTime& time)
{
	
}