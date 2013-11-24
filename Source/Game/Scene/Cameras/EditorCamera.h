// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CAMERA_EDITOR_CAMERA_
#define _ENGINE_CAMERA_EDITOR_CAMERA_

#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Point.h"

#include "Engine\Scene\Camera.h"
#include "Engine\Scene\Tickable.h"

#include "Game\UI\Scenes\Editor\UIScene_Editor.h"

#define EDITOR_CAMERA_RAYCAST_LENGTH 32.0f

class EditorGrid;

struct EditorCameraType
{
	enum Type
	{
		Projection,
		OrthoX,
		OrthoY,
		OrthoZ
	};
};

class EditorCamera : public Camera
{
private:
	float m_sensitivity_x;
	float m_sensitivity_y;
	float m_speed_x;
	float m_speed_z;
	bool  m_display_active_last_frame;

	Point m_mouse_center;

	float m_zoom;

	bool m_moving_mouse_last_frame;
	bool m_keys_down_last_frame;

	EditorCameraType::Type m_type;

	EditorMode::Type m_mode;
	UIScene_Editor* m_scene;

	EditorGrid* m_grid;
	float m_grid_offset;

public:
	EditorCamera(EditorCameraType::Type type, float fov, Rect rect, Vector3 position, Vector3 rotation);
	
	// Returns true if we are moving.
	bool IsUnderMouseControl();
	void Set_Enabled(bool value);
	void Set_Editor_Mode(EditorMode::Type type);
	void Set_Editor_Scene(UIScene_Editor* scene);
	void Set_Editor_Grid(EditorGrid* grid);
	void Update_Selection_Position();

	// Base functions.
	void Tick(const FrameTime& time);
	
	// Matricies!	
	Frustum Get_Frustum();
	Matrix4 Get_Projection_Matrix();
	Matrix4 Get_View_Matrix();
};

#endif

