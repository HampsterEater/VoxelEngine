// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_SCENE_VOXELS_CHUNK_
#define _GAME_SCENE_VOXELS_CHUNK_

#include "Engine\Engine\FrameTime.h"
#include "Engine\Renderer\Drawable.h"
#include "Engine\Renderer\Renderer.h"

#include "Generic\Types\AABB.h"
#include "Generic\Types\Sphere.h"
#include "Generic\Types\IntVector3.h"
#include "Generic\Types\LinkedList.h"

#include "Game\Scene\Voxels\Voxel.h"

class ChunkManager;
class ChunkLoader;
class ChunkUnloader;

// Determines the different states a chunk can be in.
struct ChunkStatus
{
	enum Type
	{
		Initialized,

		Pending_Load,
		Loading,
		Loaded,

		Pending_Unload,
		Unloading,
		Unloaded
	};
};

class Chunk : public Drawable
{
private:
	friend class ChunkManager;
	friend class ChunkLoader;
	friend class ChunkUnloader;
	friend class ChunkGenerator;
	friend class WorldFile;
	friend class RegionFile;

	// This contains information we use when generating a chunk's
	// mesh from all its voxels. Includes occlusion information and such.
	struct Render_Voxel
	{
	public:
		Voxel*	voxel;
		int		x, y, z;
		int		triangle_count;
		int		vertex_count;

		bool	render_top_face;
		bool	render_bottom_face;
		bool	render_left_face;
		bool	render_right_face;
		bool	render_front_face;
		bool	render_back_face;

		Render_Voxel(Voxel* pvoxel, int px, int py, int pz)
		{
			voxel = pvoxel;
			x = px;
			y = py;
			z = pz;
			render_top_face		= false;
			render_bottom_face	= false;
			render_left_face	= false;
			render_right_face	= false;
			render_front_face	= false;
			render_back_face	= false;
		}
	};

	// Voxels, yay ;_;
	Voxel* m_voxels;

	// Coordinates / Size information.
	int m_x, m_y, m_z;
	int m_width, m_height, m_depth;
	float m_voxel_width, m_voxel_height, m_voxel_depth;
	
	// AABB is cached, to speed shit up.
	AABB m_aabb;
	bool m_aabb_cached;

	// Rendering information.
	bool m_is_dirty;
	int m_mesh_id;
	int m_mesh_voxel_count;
	int m_triangle_count;

	// Store current state of chunk.
	bool m_is_empty;
	bool m_is_full;
	bool m_is_contained;

	// Status management.
	ChunkStatus::Type	m_status;
	ChunkManager*		m_manager;

	// Unload settings.
	float							m_unload_timer;
	const LinkedList<Chunk*>::Node* m_chunklist_node;

	// Flag used to determine if the chunk has a "hole" in it caused by a neighbouring chunk
	// not being loaded.
	bool m_has_hole_face;

	// Index stuff.
	__forceinline int Flatten_Index(int x, int y, int z)
	{
		return y + m_height * (x + m_depth * z);
	}

	bool Should_Render_Voxel		(Render_Voxel& voxel);
	void Regenerate_Mesh			(Renderer* renderer, bool as_neighbour = false);
	void Regenerate_Voxel			(Renderer* renderer, const Render_Voxel& voxel, int x, int y, int z);
	void Regenerate_Neighbour_Meshs	(Renderer* renderer);

	bool Is_Dirty();
	void Mark_Dirty(bool dirty = true);

	inline ChunkStatus::Type Get_Status()				{ return m_status; };
	inline void Set_Status(ChunkStatus::Type status)	{ m_status = status; };

	Voxel* Get_Voxel_Buffer()							{ return m_voxels; }
	
	int	 Get_Triangle_Count()							{ return m_triangle_count; }

	void Recalculate_State();

	bool Is_Empty()										{ return m_is_empty; }
	bool Is_Full ()										{ return m_is_full; }
	bool Is_Contained()									{ return m_is_contained; }
	bool Has_Hole_Face()								{ return m_has_hole_face; }

	// Constructors.
	Chunk();
	Chunk(ChunkManager* manager, int x, int y, int z, int width, int height, int depth, float voxel_width, float voxel_height, float voxel_depth);

public:
	~Chunk();
	
	// General properties.
	IntVector3 Get_Position() const;
	IntVector3 Get_Region() const;
	AABB Get_AABB();
	Sphere Get_Bounding_Sphere() const;
	bool Should_Render() const;

	// Unload properties.
	void Reset_Unload_Timer();
	float Get_Unload_Timer() const;

	// Getting voxels in chunk.
	Voxel* Get_Relative_Voxel(int voxel_x, int voxel_y, int voxel_z, 
							  int offset_x, int offset_y, int offset_z);
	Voxel* Get_Voxel(int x, int y, int z);

	// General manipulation of chunks.
	void Fill(VoxelType::Type type, int x, int y, int z, int width, int height, int depth);
	void Set (VoxelType::Type type, int x, int y, int z);

	// Updating / Drawing.
	int  Drawn_Voxels();
	void Draw(const FrameTime& time, RenderPipeline* pipeline);
	void Tick(const FrameTime& time);
	
};

#endif

