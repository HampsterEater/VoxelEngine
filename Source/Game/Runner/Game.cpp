// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Runner\Game.h"
#include "Game\Scene\Cameras\FirstPersonCamera.h"
#include "Game\Scene\Voxels\ChunkManager.h"

#include "Engine\Display\Display.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Engine\GameEngine.h"

#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Textures\PNG\PNGTextureFactory.h"

Game::Game(const GameConfig& config)
	: m_camera(NULL)
	, m_config(config)
{
}

const GameConfig& Game::GetConfig()
{
	return m_config;
}

void Game::Start()
{
	Display* display = Display::Get();

	// Setup camera.
	m_camera = new FirstPersonCamera(70, Rect(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	GameEngine::Get()->Get_Scene()->Add_Camera(m_camera);

	// Setup chunk manager.
	m_chunk_manager = new ChunkManager(m_config.chunk_config);
	GameEngine::Get()->Get_Scene()->Add_Drawable(m_chunk_manager);
}

void Game::End()
{
	SAFE_DELETE(m_camera);
}

void Game::Tick(const FrameTime& time)
{
	// Tick the chunk manager.
	m_chunk_manager->Tick(time);
}
