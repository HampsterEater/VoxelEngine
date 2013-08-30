// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_
#define _ENGINE_SCENE_

#include "Engine\Engine\FrameTime.h"

#include <vector>

class Camera;
class Drawable;

class Scene
{
private:
	std::vector<Camera*>   m_cameras;
	std::vector<Drawable*> m_drawables;

public:

	// Base functions.
	void Tick(const FrameTime& time);

	// Camera accessors.
	std::vector<Camera*>& Get_Cameras();
	void Add_Camera(Camera* camera);
	void Remove_Camera(Camera* camera);	
	
	// Drawable accessors.
	std::vector<Drawable*>& Get_Drawables();
	void Add_Drawable(Drawable* camera);
	void Remove_Drawable(Drawable* camera);	

};

#endif

