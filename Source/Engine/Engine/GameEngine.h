// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_GAMEENGINE_
#define _ENGINE_GAMEENGINE_

#include "Engine\Engine\FrameTime.h"
#include "Engine\Engine\GameEngineConfig.h"
#include "Engine\UI\UIManager.h"
#include "Generic\Patterns\Singleton.h"

class Renderer;
class AudioRenderer;
class Platform;
class Display;
class Scene;
class GameRunner;
class RenderPipeline;
class Locale;
class TaskManager;

class GameEngine : public Singleton<GameEngine>
{
private:
	bool				m_is_running;
	FrameTime			m_frame_time;

	GameEngineConfig	m_config;

	GameRunner*			m_runner;

	Platform*			m_platform;
	Renderer*			m_renderer;
	AudioRenderer*		m_audio_renderer;
	Display*			m_display;
	Scene*				m_scene;
	UIManager*			m_ui_manager;
	Locale*				m_locale;
	TaskManager*		m_task_manager;

	RenderPipeline*		m_render_pipeline;

public:
	~GameEngine();
	GameEngine(GameRunner* runner);
	
	const GameEngineConfig*	Get_Config();
	Scene*					Get_Scene();
	RenderPipeline*			Get_RenderPipeline();
	UIManager*				Get_UIManager();

	bool Is_Running();
	void Stop();
	void Run();

	void Tick(const FrameTime& time);

};

#endif

