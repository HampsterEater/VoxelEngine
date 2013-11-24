// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Engine\GameEngine.h"
#include "Engine\Renderer\Renderer.h"
#include "Engine\Renderer\RenderPipeline.h"
#include "Engine\Audio\AudioRenderer.h"
#include "Engine\Platform\Platform.h"
#include "Engine\Display\Display.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Localise\Locale.h"
#include "Engine\Input\Input.h"

#include "Engine\UI\Layouts\UILayoutFactory.h"

#include "Engine\Tasks\TaskManager.h"

#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Text\FontFactory.h"
#include "Engine\Audio\Sounds\SoundFactory.h"

#include "Engine\Engine\GameRunner.h"

#include "Engine\Resources\Reloadable.h"

GameEngine::GameEngine(GameRunner* runner)
	: m_is_running(true)
	, m_runner(runner)
	, m_frame_time(0)
	, m_render_pipeline(NULL)
{
	DBG_LOG("Initialising platform singleton.");
	{
		m_platform = Platform::Create();
		DBG_ASSERT(m_platform != NULL);
	}

	DBG_LOG("Performing preload.");
	{
		m_runner->Preload();
		m_config = m_runner->Get_Engine_Config();
		m_frame_time = FrameTime(m_config.target_frame_rate);
	}

	DBG_LOG("Initialising renderer.");
	{
		m_renderer = Renderer::Create();
		DBG_ASSERT(m_renderer != NULL);
	}

	DBG_LOG("Initialising display.");
	{
		m_display = Display::Create(m_config.display_title, m_config.display_width, m_config.display_height, m_config.display_fullscreen);
		DBG_ASSERT(m_display != NULL);
	}

	DBG_LOG("Binding display to renderer.");
	{
		bool result = m_renderer->Set_Display(m_display);
		DBG_ASSERT(result);
	}

	DBG_LOG("Initialising input.");
	{
		m_input = Input::Create();
		DBG_ASSERT(m_input != NULL);
	}

	DBG_LOG("Initialising rendering pipeline.");
	{
		m_render_pipeline = new RenderPipeline(m_renderer);
		DBG_ASSERT(m_render_pipeline != NULL);
	}

	DBG_LOG("Loading rendering pipeline configuration: %s", m_config.render_pipeline_file);
	{
		bool result = m_render_pipeline->Load_Config(m_config.render_pipeline_file);
		DBG_ASSERT(result);
	}
	
	DBG_LOG("Initialising audio renderer.");
	{
		m_audio_renderer = AudioRenderer::Create();
		DBG_ASSERT(m_audio_renderer != NULL);
	}

	DBG_LOG("Initialising task manager.");
	{
		m_task_manager = new TaskManager(m_config.tasks_max_workers, m_config.tasks_max_tasks);
		DBG_ASSERT(m_task_manager != NULL);
	}
		
	DBG_LOG("Loading languages.");
	{
		m_locale = Locale::Create();
		
		for (std::vector<const char*>::iterator iter = m_config.languages.begin(); iter != m_config.languages.end(); iter++)
		{
			const char* path = *iter;
			DBG_LOG("Loading language: %s", path);

			bool result = m_locale->Load_Language(path);
			DBG_ASSERT_STR(result, "Failed to load language: %s", path);
		}
	}
	
	DBG_LOG("Selecting default language: %s", m_config.language_default);
	{
		bool result = m_locale->Change_Language(m_config.language_default);
		DBG_ASSERT_STR(result, "Failed to set default language to: %s", m_config.language_default);
	}

	DBG_LOG("Setting up scene.");
	{
		m_scene = new Scene();
		DBG_ASSERT(m_scene != NULL);
	}

	DBG_LOG("Setting up UI manager.");
	{
		m_ui_manager = new UIManager();
		DBG_ASSERT(m_ui_manager != NULL);
	}
}

GameEngine::~GameEngine()
{
	// Destroy instances.
	SAFE_DELETE(m_scene);
	SAFE_DELETE(m_ui_manager);

	// Destroy resources.
	UILayoutFactory::Dispose();
	TextureFactory::Dispose();
	SoundFactory::Dispose();

	// Destroy singletons.
	TaskManager::Destroy();
	Locale::Destroy();
	RenderPipeline::Destroy();
	AudioRenderer::Destroy();
	Input::Destroy();
	Display::Destroy();
	Renderer::Destroy();
	Platform::Destroy();
}

Scene* GameEngine::Get_Scene()
{
	return m_scene;
}

UIManager* GameEngine::Get_UIManager()
{
	return m_ui_manager;
}

const GameEngineConfig*	GameEngine::Get_Config()
{
	return &m_config;
}

RenderPipeline* GameEngine::Get_RenderPipeline()
{
	return m_render_pipeline;
}

bool GameEngine::Is_Running()
{
	return m_is_running;
}

void GameEngine::Stop()
{
	m_is_running = false;

	DBG_LOG("Waiting for tasks to complete...");
	m_task_manager->Wait_For_All();
}

void GameEngine::Run()
{
	m_runner->Start();

	while (Is_Running())
	{
		m_frame_time.New_Tick();

		m_frame_time.Begin_Update();
		Tick(m_frame_time);
		m_frame_time.Finish_Update();
	
		m_frame_time.Begin_Draw();
		m_render_pipeline->Draw(m_frame_time);
		m_frame_time.Finish_Draw();
	}	

	m_runner->End();
}

void GameEngine::Tick(const FrameTime& time)
{
	// Check for resource reloads.
	Reloadable::Check_For_Reloads();

	// Tick all the main elements.
	m_input->Tick(m_frame_time);
	m_scene->Tick(time);
	m_display->Tick(time);
	m_audio_renderer->Tick(time);
	m_ui_manager->Tick(time);

	// Tick game runner.
	m_runner->Tick(time);

	// Exit on escape.
	if (m_input->Get_Keyboard_State()->Was_Key_Pressed(KeyboardKey::Escape))
	{	
		DBG_LOG("Game stopping due to escape key.");
		Stop();
	}
}