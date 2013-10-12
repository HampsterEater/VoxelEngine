// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Runner\Game.h"
#include "Game\Scene\Cameras\FirstPersonCamera.h"
#include "Game\Scene\Voxels\ChunkManager.h"

#include "Engine\Platform\Platform.h"

#include "Engine\Display\Display.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Scene\Light.h"
#include "Engine\Engine\GameEngine.h"

#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Textures\PNG\PNGTextureFactory.h"

#include "Engine\Audio\AudioRenderer.h"
#include "Engine\Audio\Sounds\Sound.h"
#include "Engine\Audio\Sounds\SoundChannel.h"
#include "Engine\Audio\Sounds\SoundFactory.h"

#include "Engine\Renderer\Shaders\Shader.h"
#include "Engine\Renderer\Shaders\ShaderFactory.h"
#include "Engine\Renderer\Shaders\ShaderProgram.h"

#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontFactory.h"
#include "Engine\Renderer\Text\FontRenderer.h"

#include "Generic\Math\Math.h"

Game::Game()
	: m_camera(NULL)
	, m_config_location("Data/Config/Game.xml")
{
}

void Game::Load_Config()
{
	DBG_LOG("Loading game configuration: %s", m_config_location);
	{
		bool result = m_config.Load(m_config_location);
		DBG_ASSERT(result);
	}

	// Create save directory structure if it dosen't exist.
	if (!Platform::Get()->Is_Directory(m_config.chunk_config.save_directory))
	{
		DBG_LOG("Creating save directory: %s", m_config.chunk_config.save_directory);
		bool ret = Platform::Get()->Create_Directory(m_config.chunk_config.save_directory, true);
		DBG_ASSERT(ret == true);
	}
	if (!Platform::Get()->Is_Directory(m_config.chunk_config.region_directory))
	{
		DBG_LOG("Creating region directory: %s", m_config.chunk_config.region_directory);
		bool ret = Platform::Get()->Create_Directory(m_config.chunk_config.region_directory, true);
		DBG_ASSERT(ret == true);
	}
}

const GameConfig& Game::Get_Config()
{
	return m_config;
}

const GameEngineConfig& Game::Get_Engine_Config()
{
	return m_config.engine_config;
}

void Game::Preload()
{
	// Load configuration information.
	Load_Config();
}

void Game::Start()
{
	Display* display = Display::Get();

	// Setup camera.
	m_camera = new FirstPersonCamera(70, Rect(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	GameEngine::Get()->Get_Scene()->Add_Camera(m_camera);
	GameEngine::Get()->Get_Scene()->Add_Tickable(m_camera);

	// Setup chunk manager.
	m_chunk_manager = new ChunkManager(m_config.chunk_config);
	GameEngine::Get()->Get_Scene()->Add_Drawable(m_chunk_manager);
	GameEngine::Get()->Get_Scene()->Add_Tickable(m_chunk_manager);

	// Random light!
	Light* light3 = new Light(LightType::Ambient, 0.0f, Color(5, 5, 5, 255));
	GameEngine::Get()->Get_Scene()->Add_Light(light3);
	
	/*
	m_directional_light = new Light(LightType::Directional, 0.0f, Color::White);
	m_directional_light->Set_Position(Vector3(0.0f, 2.0f, -4.0f));
	m_directional_light->Set_Rotation(Vector3(1.0f, 1.0f, 3.0f));
	m_directional_light->Set_Shadow_Caster(true);
	GameEngine::Get()->Get_Scene()->Add_Light(m_directional_light);
	*/

	m_directional_light = new Light(LightType::Spotlight, 10.0f, 13.0f, Color::Yellow);
	m_directional_light->Set_Position(Vector3(0.0f, 4.0f, 0.0f));
	m_directional_light->Set_Rotation(Vector3(0.0f, -DegToRad(90), 0.0f));
	m_directional_light->Set_Shadow_Caster(true);
	GameEngine::Get()->Get_Scene()->Add_Light(m_directional_light);
	
	/*
	
	Light* light = new Light(LightType::Point, 5.0f, Color::Green);
	light->Set_Position(Vector3(0.0f, 2.0f, 0.0f));
	GameEngine::Get()->Get_Scene()->Add_Light(light);
	
	Light* light2 = new Light(LightType::Point, 5.0f, Color::White);
	light2->Set_Position(Vector3(0.0f, 2.0f, -4.0f));
	GameEngine::Get()->Get_Scene()->Add_Light(light2);

	*/

	/*
	SoundHandle* sound = SoundFactory::Load("Data/Sounds/Music/zombroni.it", SoundFlags::Loop);

	SoundChannel* channel = sound->Get()->Allocate_Channel();
	sound->Get()->Play(channel, true);
	channel->Resume();
	*/
}

void Game::End()
{
	SAFE_DELETE(m_camera);
	SAFE_DELETE(m_chunk_manager);
}

void Game::Tick(const FrameTime& time)
{
	float angle = (Platform::Get()->Get_Ticks() / 1000.0f) * 1.0f;
	m_directional_light->Set_Rotation(Vector3(0.0f, angle, -DegToRad(75.0f)));
}
