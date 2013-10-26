// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Voxels\Generation\ChunkGenerator.h"
#include "Game\Scene\Voxels\ChunkManagerConfig.h"
#include "Game\Scene\Voxels\ChunkManager.h"

#include "Generic\Math\Noise\SimplexNoise.h"
#include "Generic\Math\Noise\NoiseSampler3D.h"

ChunkGenerator::ChunkGenerator(ChunkManager* manager, const ChunkManagerConfig& config)
	: m_manager(manager)
	, m_config(config)
	, m_terrain_base_noise_sampler(NULL)
	, m_terrain_base_noise(NULL)
{
	m_scale_factor = m_config.map_terrain_base_noise_sample_step;

	/*
	m_terrain_base_noise = new SimplexNoise(m_config.map_seed);
	m_terrain_base_noise_sampler = new NoiseSampler3D(m_terrain_base_noise, 
													  m_chunk_world_position,
													  IntVector3(m_config.chunk_size.X, m_config.chunk_size.Y, m_config.chunk_size.Z),
													  m_scale_factor,
													  m_config.map_terrain_base_noise_octaves,
													  m_config.map_terrain_base_noise_persistence,
													  m_config.map_terrain_base_noise_scale); 
													  */
}

ChunkGenerator::~ChunkGenerator()
{
	//SAFE_DELETE(m_terrain_base_noise_sampler);
	//SAFE_DELETE(m_terrain_base_noise);
}
	
void ChunkGenerator::Generate(Chunk* chunk)
{
	m_chunk				   = chunk;
	m_chunk_position	   = chunk->Get_Position();
	m_chunk_world_position = Vector3(m_chunk_position.X * m_config.chunk_size.X,
									 m_chunk_position.Y * m_config.chunk_size.Y,
								  	 m_chunk_position.Z * m_config.chunk_size.Z);

	// Generate noise samplers.
	//m_terrain_base_noise_sampler->Resposition(m_chunk_world_position);

	// Place base terrain.
	Place_Terrain();

	// Tell chunk to recalculate its state (if its empty etc)
	chunk->Recalculate_State();
}

void ChunkGenerator::Place_Terrain()
{
	for (int x = 0; x < m_config.chunk_size.X; x++)
	{		
		for (int y = 0; y < m_config.chunk_size.Y; y++)
		{				
			for (int z = 0; z < m_config.chunk_size.Z; z++)
			{				
				// Get a sample at the voxels position.
				//float value = m_terrain_base_noise_sampler->Sample(x, y, z);
				float value = 1.0f;

				// Bias sample by Y position. Results in up=air, down=ground
				value += (m_chunk_world_position.Y + y) * m_scale_factor.Y;
				
				// If its lower than the threshold its solid.
				if (value <= m_config.density_threshold)
				{
					m_chunk->Set(VoxelType::Solid, x, y, z);
				}
			}
		}
	}
}

