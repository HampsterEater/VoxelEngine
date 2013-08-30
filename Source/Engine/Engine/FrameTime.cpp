// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Engine\FrameTime.h"
#include "Engine\Platform\Platform.h"

FrameTime::FrameTime(int target_frame_rate)
	: m_target_frame_rate(target_frame_rate)
	, m_frame_end(0)
	, m_frame_start(0)
	, m_frame_elapsed(0)
	, m_fps(0)
	, m_fps_ticks(0)
	, m_fps_timer(0)
{
}

FrameTime::~FrameTime()
{
}

void FrameTime::New_Tick()
{
	float ticks = Platform::Get()->Get_Ticks();

	m_frame_end		= ticks;
	m_frame_elapsed = m_frame_end - m_frame_start;
	m_frame_start	= ticks;

	// Work out delta
	float tick_rate = 1000.0f / m_target_frame_rate;
	m_frame_delta = m_frame_elapsed / tick_rate;

	// Work out FPS.
	if (ticks - m_fps_timer >= 1000)
	{
		m_fps_timer = ticks;
		m_fps		= m_fps_ticks;
		m_fps_ticks = 1;
	}
	else
	{
		m_fps_ticks++;
	}
}

float FrameTime::Get_Delta() const
{
	return m_frame_delta;
}

int	FrameTime::Get_FPS() const
{
	return m_fps;
}

float FrameTime::Get_Frame_Time() const
{
	return m_frame_elapsed;
}

float FrameTime::Get_Update_Time() const
{
	return m_update_time;
}

float FrameTime::Get_Render_Time() const
{
	return m_render_time;
}

void FrameTime::Begin_Update()
{
	m_update_timer = Platform::Get()->Get_Ticks();
}

void FrameTime::Finish_Update()
{
	m_update_time = Platform::Get()->Get_Ticks() - m_update_timer;
}

void FrameTime::Begin_Draw()
{
	m_render_timer = Platform::Get()->Get_Ticks();
}

void FrameTime::Finish_Draw()
{
	m_render_time = Platform::Get()->Get_Ticks() - m_render_timer;
}
