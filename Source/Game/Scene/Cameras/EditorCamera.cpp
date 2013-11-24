// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Cameras\EditorCamera.h"
#include "Engine\Display\Display.h"
#include "Engine\Input\Input.h"
#include "Generic\Math\Math.h"

#include "Engine\Renderer\RenderPipeline.h"

#include "Engine\Engine\GameEngine.h"

#include "Game\Runner\Game.h"
#include "Game\Scene\Voxels\ChunkManager.h"

#include "Game\Scene\Editor\EditorGrid.h"

#include "Engine\UI\UIElement.h"
#include "Engine\UI\UIManager.h"

EditorCamera::EditorCamera(EditorCameraType::Type type, float fov, Rect viewport, Vector3 position, Vector3 rotation)
{
	m_fov = fov;
	m_viewport = viewport;
	m_type = type;
	m_position = position;
	m_rotation = rotation;
	m_mode = EditorMode::Pencil;
	m_scene = NULL;
	m_grid = NULL;
	m_grid_offset = 0.0f;

	m_sensitivity_x = 0.002f;
	m_sensitivity_y = 0.002f;
	m_speed_x       = 0.002f;
	m_speed_z       = 0.002f;

	if (type != EditorCameraType::Projection)
		m_zoom = 10.0f;	
	else
		m_zoom = 1.0f;	

	m_mouse_center	= Point(0, 0);

	m_moving_mouse_last_frame = false;
	m_keys_down_last_frame = false;
}

bool EditorCamera::IsUnderMouseControl()
{
	return m_moving_mouse_last_frame;
}

void EditorCamera::Set_Enabled(bool value)
{
	m_enabled = value;

	if (value == false)
	{
		m_display_active_last_frame = false;
		m_moving_mouse_last_frame = false;
	}
}

void EditorCamera::Set_Editor_Mode(EditorMode::Type type)
{
	m_mode = type;
}

void EditorCamera::Set_Editor_Scene(UIScene_Editor* scene)
{
	m_scene = scene;
}

void EditorCamera::Set_Editor_Grid(EditorGrid* grid)
{
	m_grid = grid;
}

void EditorCamera::Update_Selection_Position()
{
	Input*				input			= Input::Get();
	KeyboardState*		keyboard		= input->Get_Keyboard_State();
	MouseState*			mouse			= input->Get_Mouse_State();
	Point				position		= mouse->Get_Position();
	ChunkManager*		manager			= Game::Get()->Get_Chunk_Manager();
	Game*				game		= Game::Get();
	const GameConfig&	config		= game->Get_Config();

	Vector3			near_position	= Unproject(Vector3(position.X, position.Y, 0.0f));
	Vector3			far_position	= Unproject(Vector3(position.X, position.Y, 1.0f)); 
	Vector3			direction		= (far_position - near_position).Normalize();

	std::vector<ChunkRaycastResult>	results;
	ChunkRaycastBoundries boundries;

	switch (m_type)
	{
	case EditorCameraType::Projection:
		boundries.Use_YBoundry = true;
		boundries.YBoundry	   = floorf(m_grid_offset / config.chunk_config.voxel_size.Y);
		break;

	case EditorCameraType::OrthoX:
		boundries.Use_XBoundry = true;
		boundries.XBoundry	   = 0;
		boundries.XBoundry	   = floorf(m_grid_offset / config.chunk_config.voxel_size.X);
		break;

	case EditorCameraType::OrthoY:
		boundries.Use_YBoundry = true;
		boundries.YBoundry	   = 0;
		boundries.YBoundry	   = floorf(m_grid_offset / config.chunk_config.voxel_size.Y);
		break;

	case EditorCameraType::OrthoZ:
		boundries.Use_ZBoundry = true;
		boundries.ZBoundry	   = 0;
		boundries.ZBoundry	   = floorf(m_grid_offset / config.chunk_config.voxel_size.Z);
		break;
	}

	int count = manager->Raycast(near_position, near_position + (direction * EDITOR_CAMERA_RAYCAST_LENGTH), results, 1, &boundries);
	if (count > 0)
	{
		ChunkRaycastResult result = results.at(0);
		m_scene->Set_Selection_Position(result.AbsoluteVoxel, result.Normal);
	}
}

void EditorCamera::Tick(const FrameTime& time)
{
	// Get basic variables.
	Display*			display		= Display::Get();
	Input*				input		= Input::Get();
	KeyboardState*		keyboard	= input->Get_Keyboard_State();
	MouseState*			mouse		= input->Get_Mouse_State();
	UIManager*			uimanager	= GameEngine::Get()->Get_UIManager();
	Game*				game		= Game::Get();
	const GameConfig&	config		= game->Get_Config();

	int				display_width	= display->Get_Width();
	int				display_height	= display->Get_Height();
	float			delta			= time.Get_Delta();

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
	Vector3 up = right.Cross(direction);
	
	// Work out how far the mouse moved.
	Point mouse_position = mouse->Get_Position();
	float mouse_delta_x = mouse_position.X - m_mouse_center.X;
	float mouse_delta_y = mouse_position.Y - m_mouse_center.Y;

	// Are we active?
	bool in_viewport = m_viewport.Intersects(mouse_position);
	bool active		 = display->Is_Active() && ((in_viewport && uimanager->Find_Element_By_Position(mouse_position, true) == NULL) || m_moving_mouse_last_frame == true);
	bool keys_down	 = (mouse->Is_Button_Down(MouseButton::Left) || mouse->Is_Button_Down(MouseButton::Right));
	bool ortho		 = (m_type != EditorCameraType::Projection);
	bool lshift_down = keyboard->Is_Key_Down(KeyboardKey::LeftShift);
	bool lctrl_down	 = keyboard->Is_Key_Down(KeyboardKey::LeftCtrl);

	// Disable movement if we are not in an editable tool mode.
	if (!lshift_down && !lctrl_down)
	{
		active = false;
	}
	
	// Calculate mouse center.
	m_mouse_center = Point(floorf(m_viewport.X + (m_viewport.Width * 0.5f)), floorf(m_viewport.Y + (m_viewport.Height * 0.5f)));

	// Update selection position.
	if (in_viewport == true && (keys_down == false || (!lshift_down && !lctrl_down)))
	{
		Update_Selection_Position();
	}

	// Camera control.
	if (active == true)
	{
		if (m_display_active_last_frame == true)
		{
			// Move camera up/down absolute.
			if (lshift_down == true && mouse->Is_Button_Down(MouseButton::Left) && mouse->Is_Button_Down(MouseButton::Right) && ortho == false)
			{
				m_position.Y -= mouse_delta_y * m_sensitivity_y;
			}

			// Move camera forward/backward at constant height.
			else if (mouse->Is_Button_Down(MouseButton::Left))
			{			
				// Move grid up/down.
				if (lctrl_down)
				{
					if (m_grid != NULL)
					{
						m_grid_offset -= (mouse_delta_y * m_sensitivity_y);

						Vector3 position = m_grid->Get_Position();

						switch (m_type)
						{
						case EditorCameraType::Projection:
						case EditorCameraType::OrthoY:
							position.Y = floorf(m_grid_offset / config.chunk_config.voxel_size.Y) * config.chunk_config.voxel_size.Y;
							break;

						case EditorCameraType::OrthoX:
							position.X = floorf(m_grid_offset / config.chunk_config.voxel_size.X) * config.chunk_config.voxel_size.X;
							break;

						case EditorCameraType::OrthoZ:
							position.Z = floorf(m_grid_offset / config.chunk_config.voxel_size.Z) * config.chunk_config.voxel_size.Z;
							break;
						}
						m_grid->Set_Position(position);
					}
				}
				else if (lshift_down == true)
				{
					float old_y = m_position.Y;

					if (ortho == false)
					{
						m_position = m_position + (direction * (mouse_delta_y * m_speed_z) * delta);
						m_position = m_position - (right * (mouse_delta_x * m_speed_x) * delta);
						m_position.Y = old_y;
					}
					else
					{
						m_position = m_position + (up * (mouse_delta_y * m_speed_z) * delta);
						m_position = m_position - (right * (mouse_delta_x * m_speed_x) * delta);
					}
				}
			}

			// Pan camera.
			else if (lshift_down == true && mouse->Is_Button_Down(MouseButton::Right))
			{
				if (ortho == false)
				{
					// Turn camera if this is not the first run.
					m_rotation.Y -= (mouse_delta_x * m_sensitivity_x);
					m_rotation.Z -= mouse_delta_y * m_sensitivity_y;

					// Don't want any kind of upside down shenanigans.
					float deg90 = DegToRad(90);
					m_rotation.Z = Clamp(m_rotation.Z, -deg90, deg90);
				}
				else
				{
					m_zoom = Max(0.1f, m_zoom + (mouse_delta_y * m_speed_z));
				}
			}

			// No mouse :(
			else 
			{
				mouse->Set_Position(m_mouse_center);
				m_moving_mouse_last_frame = false;
				m_display_active_last_frame = false;
			}
		}
		else
		{
			if (keys_down == true && m_keys_down_last_frame == false)
			{
				m_display_active_last_frame = true;
			}
		}
	}

	// Reset mouse position.
	if (active == true && m_display_active_last_frame == true)
	{		
		mouse->Set_Position(m_mouse_center);
		m_moving_mouse_last_frame = true;
	}
	m_keys_down_last_frame = keys_down;
}

Frustum EditorCamera::Get_Frustum()
{
	Rect	  viewport = Get_Viewport();
	Vector3   rotation = Get_Rotation();
	Vector3   position = Get_Position();

	// Calculate view matrix.
	float horizontal = rotation.Y;
	float vertical   = rotation.Z;
	Vector3 direction
	(
		cos(vertical) * sin(horizontal),
		sin(vertical),
		cos(vertical) * cos(horizontal)
	);
	Vector3	right
	(
		sin(horizontal - 3.14f / 2.0f),
		0,
		cos(horizontal - 3.14f / 2.0f)
	);
	Vector3 center = position + direction;
	Vector3 up = right.Cross(direction);
		
	float x_zoom = m_zoom;
	float y_zoom = m_zoom;

	if (viewport.Width > viewport.Height)
	{
		x_zoom *= (viewport.Width / viewport.Height);
	}
	else
	{
		y_zoom *= (viewport.Height / viewport.Width);
	}

	switch (m_type)
	{
	case EditorCameraType::Projection:	
		return Frustum(m_fov,
					   viewport.Width / (float) viewport.Height,
					   m_near_clip,
					   m_far_clip,
					   position,
					   center,
					   up);
	
	case EditorCameraType::OrthoX:
	case EditorCameraType::OrthoY:
	case EditorCameraType::OrthoZ:
		return Frustum(Point(1 * x_zoom, 1 * y_zoom),
						m_near_clip,
						m_far_clip,
						position,
						center,
						up);	
	}
}

Matrix4 EditorCamera::Get_Projection_Matrix()
{
	Rect  viewport	= Get_Viewport();
	float near_clip	= Get_Near_Clip();
	float far_clip	= Get_Far_Clip();

	float aspect = viewport.Width / viewport.Height;

	float x_zoom = m_zoom;
	float y_zoom = m_zoom;

	if (viewport.Width > viewport.Height)
	{
		x_zoom *= (viewport.Width / viewport.Height);
	}
	else
	{
		y_zoom *= (viewport.Height / viewport.Width);
	}

	switch (m_type)
	{
	case EditorCameraType::Projection:	
		return Matrix4::Perspective(Get_FOV(), aspect, near_clip, far_clip);

	case EditorCameraType::OrthoX:
	case EditorCameraType::OrthoY:
	case EditorCameraType::OrthoZ:
		return Matrix4::Orthographic(-1 * x_zoom, 1 * x_zoom, -1 * y_zoom, 1 * y_zoom, near_clip, far_clip);
	}
}

Matrix4 EditorCamera::Get_View_Matrix()
{
	Vector3 rotation = Get_Rotation();
	Vector3 position = Get_Position();

	// Calculate view matrix.
	float horizontal = rotation.Y;
	float vertical   = rotation.Z;
	Vector3 direction
	(
		cos(vertical) * sin(horizontal),
		sin(vertical),
		cos(vertical) * cos(horizontal)
	);
	Vector3	right
	(
		sin(horizontal - 3.14f / 2.0f),
		0,
		cos(horizontal - 3.14f / 2.0f)
	);
	Vector3 center = position + direction;
	Vector3 up = right.Cross(direction);
	
	return Matrix4::LookAt(position, center, up);
}