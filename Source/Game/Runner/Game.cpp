// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Runner\Game.h"
#include "Game\Scene\Cameras\FirstPersonCamera.h"
#include "Game\Scene\Cameras\GameCamera.h"
#include "Game\Scene\Cameras\EditorCamera.h"
#include "Game\Scene\Cameras\OrthoCamera.h"
#include "Game\Scene\Voxels\ChunkManager.h"

#include "Game\Scene\Editor\EditorGrid.h"
#include "Game\Scene\Editor\EditorSelectionPrimitive.h"

#include "Engine\Platform\Platform.h"

#include "Engine\Display\Display.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Scene\Light.h"
#include "Engine\Engine\GameEngine.h"

#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Textures\PNG\PNGPixmapFactory.h"

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

#include "Game\UI\Scenes\UIScene_Game.h"

#include "Game\Version.h"

#include "Generic\Math\Math.h"

void Print_Game_Version()
{
#ifdef DEBUG_BUILD
	DBG_LOG(" Dat Rougelike Debug    Version %s, Built %s/%s/%s %s:%s", GameAutoVersion::FULLVERSION_STRING, GameAutoVersion::DAY, GameAutoVersion::MONTH, GameAutoVersion::YEAR, GameAutoVersion::HOUR, GameAutoVersion::MINUTE);
#else
	DBG_LOG(" Dat Rougelike Release  Version %s, Built %s/%s/%s %s:%s", GameAutoVersion::FULLVERSION_STRING, GameAutoVersion::DAY, GameAutoVersion::MONTH, GameAutoVersion::YEAR, GameAutoVersion::HOUR, GameAutoVersion::MINUTE);
#endif
}

GameRunner* New_Game()
{
	return new Game();
}

Game::Game()
	: m_config_location("Data/Config/Game.xml")
{
	memset(m_cameras, 0, sizeof(Camera*) * CameraID::COUNT);
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

ChunkManager* Game::Get_Chunk_Manager()
{
	return m_chunk_manager;
}

EditorSelectionPrimitive* Game::Get_Editor_Mouse_Selection_Primitive()
{
	return m_editor_mouse_selection_primitive;
}

EditorSelectionPrimitive* Game::Get_Editor_Main_Selection_Primitive()
{
	return m_editor_main_selection_primitive;
}

void Game::Preload()
{
	// Load configuration information.
	Load_Config();
}

void Game::Setup_Cameras()
{
	Display* display = Display::Get();
	GameEngine* engine = GameEngine::Get();

	float distance = 5.0f;
	float rotation = -DegToRad(90.0f);

	// Setup cameras.
	m_cameras[CameraID::Game]			= new GameCamera  (70, Rect(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	m_cameras[CameraID::UI]				= new OrthoCamera (70, Rect(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()));
	m_cameras[CameraID::Editor_Main]	= new EditorCamera(EditorCameraType::Projection, 70, Rect(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()), Vector3(0, 0, 0), Vector3(0, 0, 0));
	m_cameras[CameraID::Editor_OrthoX]	= new EditorCamera(EditorCameraType::OrthoX, 70, Rect(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()), Vector3(distance, 0, 0), Vector3(-rotation, rotation, 0));
	m_cameras[CameraID::Editor_OrthoY]	= new EditorCamera(EditorCameraType::OrthoY, 70, Rect(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()), Vector3(0, distance, 0), Vector3(0, 0, rotation));
	m_cameras[CameraID::Editor_OrthoZ]	= new EditorCamera(EditorCameraType::OrthoZ, 70, Rect(0.0f, 0.0f, (float)display->Get_Width(), (float)display->Get_Height()), Vector3(0, 0, -distance), Vector3(0, 0, 0));

	m_cameras[CameraID::Editor_Main]->Set_Enabled(false);
	m_cameras[CameraID::Editor_OrthoX]->Set_Enabled(false);
	m_cameras[CameraID::Editor_OrthoY]->Set_Enabled(false);
	m_cameras[CameraID::Editor_OrthoZ]->Set_Enabled(false);

	// Add cameras to scene.
	for (int i = 0; i < CameraID::COUNT; i++)
	{
		GameEngine::Get()->Get_Scene()->Add_Camera(m_cameras[i]);
		GameEngine::Get()->Get_Scene()->Add_Tickable(m_cameras[i]);
	}

	// Set HUD camera.
	engine->Get_UIManager()->Set_Camera(m_cameras[CameraID::UI]);
}

void Game::Setup_Editor()
{
	EditorCamera* editor_main_camera   = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Main));
	EditorCamera* editor_orthox_camera = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_OrthoX));
	EditorCamera* editor_orthoy_camera = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_OrthoY));
	EditorCamera* editor_orthoz_camera = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_OrthoZ));
	Scene* scene = GameEngine::Get()->Get_Scene();

	// Projection grid.
	m_editor_projection_grid = new EditorGrid();
	m_editor_projection_grid->Set_Render_Slot("solid_color_editor_geometry");
	scene->Add_Drawable(m_editor_projection_grid);
	scene->Add_Tickable(m_editor_projection_grid);
	m_editor_projection_grid->Set_Draw_Camera(editor_main_camera);
	editor_main_camera->Set_Editor_Grid(m_editor_projection_grid);

	// Side-on (X) grid.
	m_editor_orthox_grid = new EditorGrid();
	m_editor_orthox_grid->Set_Render_Slot("solid_color_editor_geometry");
	m_editor_orthox_grid->Set_Rotation(Vector3(0, 0, DegToRad(90)));
	scene->Add_Drawable(m_editor_orthox_grid);
	scene->Add_Tickable(m_editor_orthox_grid);
	m_editor_orthox_grid->Set_Draw_Camera(editor_orthox_camera);
	editor_orthox_camera->Set_Editor_Grid(m_editor_orthox_grid);
	
	// Side-on (Z) grid.
	m_editor_orthoy_grid = new EditorGrid();
	m_editor_orthoy_grid->Set_Render_Slot("solid_color_editor_geometry");
	m_editor_orthoy_grid->Set_Rotation(Vector3(DegToRad(90), 0, 0));
	scene->Add_Drawable(m_editor_orthoy_grid);
	scene->Add_Tickable(m_editor_orthoy_grid);
	m_editor_orthoy_grid->Set_Draw_Camera(editor_orthoz_camera);
	editor_orthoz_camera->Set_Editor_Grid(m_editor_orthoy_grid);

	// Top-down grid.
	m_editor_orthoz_grid = new EditorGrid();
	m_editor_orthoz_grid->Set_Render_Slot("solid_color_editor_geometry");
	scene->Add_Drawable(m_editor_orthoz_grid);
	scene->Add_Tickable(m_editor_orthoz_grid);
	m_editor_orthoz_grid->Set_Draw_Camera(editor_orthoy_camera);
	editor_orthoy_camera->Set_Editor_Grid(m_editor_orthoz_grid);

	// Mouse selector primitive.
	m_editor_mouse_selection_primitive = new EditorSelectionPrimitive();
	m_editor_mouse_selection_primitive->Set_Render_Slot("solid_color_editor_geometry");
	scene->Add_Drawable(m_editor_mouse_selection_primitive);
	scene->Add_Tickable(m_editor_mouse_selection_primitive);	

	// Main selector primitive (cube/sphere/etc).
	m_editor_main_selection_primitive = new EditorSelectionPrimitive();
	m_editor_main_selection_primitive->Set_Render_Slot("solid_color_editor_geometry");
	scene->Add_Drawable(m_editor_main_selection_primitive);
	scene->Add_Tickable(m_editor_main_selection_primitive);
}

void Game::Start()
{
	Display* display = Display::Get();

	// Setup cameras.
	Setup_Cameras();

	// Setup editor related stuff.
	Setup_Editor();

	// Setup chunk manager.
	m_chunk_manager = new ChunkManager(m_config.chunk_config);
	m_chunk_manager->Set_Render_Slot("geometry");
	GameEngine::Get()->Get_Scene()->Add_Drawable(m_chunk_manager);
	GameEngine::Get()->Get_Scene()->Add_Tickable(m_chunk_manager);


	// -----------------------------------------------------------------------------------------
	// Testing shit
	// -----------------------------------------------------------------------------------------

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

	// -----------------------------------------------------------------------------------------
	// Testing shit
	// -----------------------------------------------------------------------------------------

	// Push the game state.
	UIManager* manager = GameEngine::Get()->Get_UIManager();
	manager->Push(new UIScene_Game());
}

Camera* Game::Get_Camera(CameraID::Type camera)
{
	return m_cameras[(int)camera];
}

void Game::End()
{
	for (int i = 0; i < CameraID::COUNT; i++)
	{
		SAFE_DELETE(m_cameras[i]);
	}

	SAFE_DELETE(m_chunk_manager);
}

void Game::Tick(const FrameTime& time)
{
	float angle = (Platform::Get()->Get_Ticks() / 1000.0f) * 1.0f;
	m_directional_light->Set_Rotation(Vector3(0.0f, angle, -DegToRad(75.0f)));
}
