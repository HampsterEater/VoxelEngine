// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Platform\Win32\Entry.h"

#include "Generic\Threads\Thread.h"

#include "Game\Runner\Game.h"
#include "Game\Runner\GameConfig.h"

#include "Engine\Engine\GameEngineConfig.h"
#include "Engine\Engine\GameEngine.h"

#include "Engine\Renderer\Textures\PNG\PNGTextureFactory.h"
#include "Engine\IO\Win32\Win32_FileStreamFactory.h"

#include <Windows.h>

int main(int argc, char* argv[])
{	
	// Instantiate factories we will be using. (I wish static instantiation for auto-registration worked
	//											over static library boundries ;_;)
	Win32_FileStreamFactory fileStreamFactory;
	PNGTextureFactory		pngFactory;

	// Set main thread affinity to the first core only.
	Thread::Get_Current()->Set_Affinity(1 << 0);

	// Seed random.
	srand(GetTickCount());

	// Set configuration options.
	GameConfig config;
	config.engine_config.target_frame_rate					= 60;
	config.engine_config.display_width						= 640;
	config.engine_config.display_height						= 480;
	config.engine_config.display_fullscreen					= false;
	config.engine_config.display_title						= "Game Engine Test";
	
	config.chunk_config.chunk_size							= IntVector3(16,	16,		16);
	config.chunk_config.voxel_size							= Vector3	(0.1f,	0.1f,	0.1f);
	config.chunk_config.draw_distance						= IntVector3(16,	16,		16);
	config.chunk_config.load_distance						= IntVector3(17,	17,		17);
	config.chunk_config.unload_distance						= IntVector3(18,	18,		18);
	config.chunk_config.chunk_regenerations_per_frame		= 20;
	config.chunk_config.chunk_unloads_per_frame				= 50;
	config.chunk_config.unload_timeout						= 1000.0f * 10;

	config.chunk_config.chunk_memory_pool_buffer			= 1.6f;
	config.chunk_config.voxel_memory_pool_buffer			= 1.6f;

	config.chunk_config.map_seed							= rand();
	config.chunk_config.density_threshold					= 0.0f;
	config.chunk_config.map_terrain_base_noise_sample_step	= Vector3(0.02f, 0.06f, 0.02f);
	config.chunk_config.map_terrain_base_noise_octaves		= 2;
	config.chunk_config.map_terrain_base_noise_persistence	= 0.5f;
	config.chunk_config.map_terrain_base_noise_scale		= 0.5f;

	// Run the game.
	Game runner(config);
	GameEngine engine(config.engine_config, &runner);
	engine.Run();
}