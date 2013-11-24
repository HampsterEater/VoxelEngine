// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CAMERA_GAME_CAMERA_
#define _ENGINE_CAMERA_GAME_CAMERA_

#include "Generic\Types\Rectangle.h"

#include "Engine\Scene\Camera.h"
#include "Engine\Scene\Tickable.h"

class GameCamera : public Camera
{
private:
	float m_sensitivity_x;
	float m_sensitivity_y;
	float m_speed_x;
	float m_speed_z;
	bool  m_display_active_last_frame;
	bool  m_active;

public:
	GameCamera(float fov, Rect rect);
	
	void Set_Enabled(bool value);

	// Base functions.
	void Tick(const FrameTime& time);

};

#endif

