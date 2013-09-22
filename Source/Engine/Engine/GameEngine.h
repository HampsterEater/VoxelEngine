// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_GAMEENGINE_
#define _ENGINE_GAMEENGINE_

#include "Engine\Engine\FrameTime.h"
#include "Engine\Engine\GameEngineConfig.h"
#include "Generic\Patterns\Singleton.h"

class Renderer;
class Platform;
class Display;
class Scene;
class GameRunner;
class RenderPipeline;

class GameEngine : public Singleton<GameEngine>
{
private:
	bool				m_is_running;
	FrameTime			m_frame_time;

	GameEngineConfig	m_config;

	GameRunner*			m_runner;

	Platform*			m_platform;
	Renderer*			m_renderer;
	Display*			m_display;
	Scene*				m_scene;

	RenderPipeline*		m_render_pipeline;

public:
	~GameEngine();
	GameEngine(GameRunner* runner);

	Scene*			Get_Scene();
	RenderPipeline* Get_RenderPipeline();

	bool Is_Running();
	void Stop();
	void Run();

	void Tick(const FrameTime& time);

};

#endif

