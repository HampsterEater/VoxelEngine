// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CAMERA_
#define _ENGINE_CAMERA_

#include "Generic\Types\Rectangle.h"

#include "Engine\Engine\FrameTime.h"
#include "Engine\Scene\Actor.h"

class Camera : public Actor
{
protected:
	Rect	m_viewport;
	float	m_fov;

public:

	// Constructors.
	Camera();

	// Get modifiers.
	virtual Rect Get_Viewport();
	virtual void Set_Viewport(Rect rect);

	virtual float Get_FOV();
	virtual void  Set_FOV(float fov);

};

#endif

