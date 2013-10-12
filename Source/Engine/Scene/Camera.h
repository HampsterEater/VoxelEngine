// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CAMERA_
#define _ENGINE_CAMERA_

#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Frustum.h"
#include "Generic\Types\Matrix4.h"

#include "Engine\Engine\FrameTime.h"
#include "Engine\Scene\Actor.h"
#include "Engine\Scene\Tickable.h"

class Camera : public Actor, public Tickable
{
protected:
	Rect	m_viewport;
	float	m_fov;
	float	m_near_clip;
	float	m_far_clip;

public:

	// Constructors.
	Camera();

	// Get modifiers.
	Frustum Get_Frustum();

	virtual Rect Get_Viewport();
	virtual void Set_Viewport(Rect rect);

	virtual float Get_FOV();
	virtual void  Set_FOV(float fov);

	virtual float Get_Near_Clip();
	virtual void  Set_Near_Clip(float clip);

	virtual float Get_Far_Clip();
	virtual void  Set_Far_Clip(float clip);

	virtual Matrix4 Get_Projection_Matrix();
	virtual Matrix4 Get_View_Matrix();

};

#endif

