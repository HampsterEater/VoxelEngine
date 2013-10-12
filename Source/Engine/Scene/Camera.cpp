// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Scene\Camera.h"
#include "Engine\Platform\Platform.h"

Camera::Camera() 
	: m_fov(60)
	, m_viewport(0, 0, 800, 600)
	, m_near_clip(0.1f)
	, m_far_clip(18.0f)
{
}

Rect Camera::Get_Viewport()
{
	return m_viewport;
}

void Camera::Set_Viewport(Rect rect)
{
	m_viewport = rect;
}

float Camera::Get_FOV()
{
	return m_fov;
}

void Camera::Set_FOV(float fov)
{
	m_fov = fov;
}

float Camera::Get_Near_Clip()
{
	return m_near_clip;
}

void Camera::Set_Near_Clip(float clip)
{
	m_near_clip = clip;
}

float Camera::Get_Far_Clip()
{
	return m_far_clip;
}

void Camera::Set_Far_Clip(float clip)
{
	m_far_clip = clip;
}

Frustum Camera::Get_Frustum()
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
	
	return Frustum(m_fov,
				   viewport.Width / (float) viewport.Height,
				   m_near_clip,
				   m_far_clip,
				   position,
				   center,
				   up);
}

Matrix4 Camera::Get_Projection_Matrix()
{
	Rect  viewport	= Get_Viewport();
	float near_clip	= Get_Near_Clip();
	float far_clip	= Get_Far_Clip();

	return Matrix4::Perspective(Get_FOV(), viewport.Width / viewport.Height, near_clip, far_clip);
}

Matrix4 Camera::Get_View_Matrix()
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
