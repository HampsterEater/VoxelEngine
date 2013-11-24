// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Cameras\FirstPersonCamera.h"
#include "Engine\Display\Display.h"
#include "Engine\Input\Input.h"
#include "Generic\Math\Math.h"

#include "Engine\Renderer\RenderPipeline.h"

FirstPersonCamera::FirstPersonCamera(float fov, Rect viewport)
{
	m_fov = fov;
	m_viewport = viewport;

	m_sensitivity_x = 0.002f;
	m_sensitivity_y = 0.002f;
	m_speed_x       = 0.05f;
	m_speed_z       = 0.05f;
	m_display_active_last_frame = false;
}

void FirstPersonCamera::Tick(const FrameTime& time)
{
	// Don't update if we are not the active camera.
	if (this != RenderPipeline::Get()->Get_Active_Camera())
	{
		m_display_active_last_frame = false;
		return;
	}

	Display*		display		= Display::Get();
	Input*			input		= Input::Get();
	KeyboardState*	keyboard	= input->Get_Keyboard_State();
	MouseState*		mouse		= input->Get_Mouse_State();

	int display_width = display->Get_Width();
	int display_height = display->Get_Height();
	float delta = time.Get_Delta();

	// Calculate some important vectors!
	float horizontal = m_rotation.Y;
	float vertical   = m_rotation.Z;
	Vector3 direction
	(
		cos(vertical) * sin(horizontal),
		sin(vertical),
		cos(vertical) * cos(horizontal)
	);
	Vector3	right
	(
		sin(m_rotation.Y - 3.14f / 2.0f),
		0,
		cos(m_rotation.Y - 3.14f / 2.0f)
	);

	// Camera rotation.
	{
		if (display->Is_Active())
		{
			if (m_display_active_last_frame == true)
			{
				// Work out how far the mouse moved.
				Point mouse_position = mouse->Get_Position();
				float mouse_delta_x = mouse_position.X - (display_width / 2);
				float mouse_delta_y = mouse_position.Y - (display_height / 2);

				// Turn camera if this is not the first run.
				m_rotation.Y -= (mouse_delta_x * m_sensitivity_x);
				m_rotation.Z -= mouse_delta_y * m_sensitivity_y;

				// Don't want any kind of upside down shenanigans.
				float deg90 = DegToRad(90);
				m_rotation.Z = Clamp(m_rotation.Z, -deg90, deg90);
			}
			
			mouse->Set_Position(Point(display_width / 2, display_height / 2));
			m_display_active_last_frame = true;
		}
		else
		{
			m_display_active_last_frame = false;
		}
	}

	// Camera movement.
	{
		Vector3 velocity(0.0f, 0.0f, 0.0f);

		if (keyboard->Is_Key_Down(KeyboardKey::A))
		{
			m_position = m_position - (right * m_speed_x * delta);
		}
		if (keyboard->Is_Key_Down(KeyboardKey::D))
		{
			m_position = m_position + (right * m_speed_x * delta);
		}
		if (keyboard->Is_Key_Down(KeyboardKey::W))
		{
			m_position = m_position + (direction * m_speed_z * delta);
		}
		if (keyboard->Is_Key_Down(KeyboardKey::S))
		{
			m_position = m_position - (direction * m_speed_z * delta);
		}

		m_position = m_position + velocity.Cross(m_rotation);
	}
}