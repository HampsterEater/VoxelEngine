// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Engine\GameEngine.h"
#include "Engine\Renderer\Renderer.h"
#include "Engine\Renderer\RenderPipeline.h"
#include "Engine\Platform\Platform.h"
#include "Engine\Display\Display.h"
#include "Engine\Scene\Scene.h"

#include "Engine\Engine\GameRunner.h"

GameEngine::GameEngine(GameRunner* runner)
	: m_is_running(true)
	, m_runner(runner)
	, m_frame_time(0)
	, m_render_pipeline(NULL)
{
	DBG_LOG("=============================================================");
	DBG_LOG(" Voxel Engine Test");
	DBG_LOG("=============================================================");
	DBG_LOG(" Copyright (C) 2013 Tim Leonard");
	DBG_LOG(" All Rights Reserved");
	DBG_LOG("=============================================================");

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

	DBG_LOG("Setting up scene.");
	{
		m_scene = new Scene();
		DBG_ASSERT(m_scene != NULL);
	}
}

GameEngine::~GameEngine()
{
	SAFE_DELETE(m_scene);
	SAFE_DELETE(m_display);
	SAFE_DELETE(m_renderer);
	SAFE_DELETE(m_platform);
}

Scene* GameEngine::Get_Scene()
{
	return m_scene;
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
	// Tick everything.
	m_scene->Tick(time);
	m_display->Tick(time);
	m_render_pipeline->Tick(time);

	// Tick game runner.
	m_runner->Tick(time);

	// Exit on escape.
	if (m_display->Is_Key_Pressed(Key::Escape))
	{	
		DBG_LOG("Game stopping due to escape key.");
		Stop();
	}
}