// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Scene\Camera.h"

Camera::Camera() 
	: m_fov(60)
	, m_viewport(0, 0, 640, 480)
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
