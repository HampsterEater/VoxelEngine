// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CAMERA_ORTHO_CAMERA_
#define _ENGINE_CAMERA_ORTHO_CAMERA_

#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Point.h"

#include "Engine\Scene\Camera.h"
#include "Engine\Scene\Tickable.h"

class OrthoCamera : public Camera
{
private:

public:
	OrthoCamera(float fov, Rect rect);
	
	// Base functions.
	void Tick(const FrameTime& time);
	
	// Projection matricies.
	Matrix4 Get_Projection_Matrix();
	Matrix4 Get_View_Matrix();
	
};

#endif

