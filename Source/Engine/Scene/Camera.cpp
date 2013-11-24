// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Scene\Camera.h"
#include "Engine\Platform\Platform.h"

#include "Engine\Display\Display.h"

#include "Generic\Math\Math.h"

Camera::Camera() 
	: m_fov(60)
	, m_viewport(0, 0, 800, 600)
	, m_near_clip(0.1f)
	, m_far_clip(40.0f)
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

Vector3 Camera::Unproject(Vector3 position)
{
	Display* display = Display::Get();

	// Calculate viewport.
	Rect viewport	= m_viewport; 
	viewport.X		= viewport.X;
	viewport.Y		= viewport.Y;
	viewport.Width	= Max(viewport.Width, 0);
	viewport.Height	= Max(viewport.Height, 0);

	// Get inverse transform of projection*view matrix.
	Matrix4 inverseTransform = (Get_Projection_Matrix() * Get_View_Matrix()).Inverse();

	// Transformation to normalised coordinates.
	Vector4 inVector = Vector4
	(
		((position.X - viewport.X) / viewport.Width) * 2.0 - 1.0,
		-((position.Y - viewport.Y) / viewport.Height) * 2.0 + 1.0,
		2.0 * position.Z - 1.0,
		1.0
	);

	// Transform to world coordinates.
	Vector4 outVector = inverseTransform * inVector;
	if (outVector.W == 0.0)
	{
		return Vector3(0, 0, 0);
	}

	return Vector3 
	(
		outVector.X / outVector.W,
		outVector.Y / outVector.W,
		outVector.Z / outVector.W
	);
}
