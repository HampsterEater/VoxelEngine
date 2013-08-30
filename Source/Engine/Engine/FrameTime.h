// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_FRAMETIME_
#define _ENGINE_FRAMETIME_

class FrameTime
{
public:
	FrameTime(int target_frame_rate);
	~FrameTime();

	void	New_Tick();
	float	Get_Delta()			const;
	int		 Get_FPS()			const;
	float	 Get_Frame_Time()	const;
	float	 Get_Update_Time()	const;
	float	 Get_Render_Time()	const;

	void	Begin_Update();
	void	Finish_Update();
	void	Begin_Draw();
	void	Finish_Draw();

private:
	int		m_target_frame_rate;
	
	float	m_update_timer;
	float	m_update_time;

	float	m_render_timer;
	float	m_render_time;

	float	m_frame_start;
	float	m_frame_end;
	float	m_frame_elapsed;
	float	m_frame_delta;

	int		m_fps;
	int		m_fps_ticks;
	int		m_fps_timer;

};

#endif

