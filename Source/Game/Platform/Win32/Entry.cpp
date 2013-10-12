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
#include "Engine\Renderer\Text\FreeType\FreeType_FontFactory.h"
#include "Engine\IO\Win32\Win32_FileStreamFactory.h"
#include "Engine\Audio\FMod\FMod_SoundFactory.h"

#include <Windows.h>
#include <cstdio>

int main(int argc, char* argv[])
{	
	// Instantiate factories we will be using. (I wish static instantiation for auto-registration worked
	//											over static library boundries ;_;)
	Win32_FileStreamFactory fileStreamFactory;
	PNGTextureFactory		pngFactory;
	FMod_SoundFactory		fmodFactory;
	FreeType_FontFactory	freeTypeFactory;

	// Set main thread affinity to the first core only.
//	Thread::Get_Current()->Set_Affinity(1 << 0);

	// Seed random.
	srand(GetTickCount());

	// Run the game.
	Game* runner = new Game();
	GameEngine engine(runner);
	engine.Run();
}