// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_
#define _ENGINE_SCENE_

#include "Engine\Engine\FrameTime.h"

#include <vector>

class Camera;
class Drawable;
class Tickable;
class Light;

class Scene
{
private:
	std::vector<Camera*>   m_cameras;
	std::vector<Drawable*> m_drawables;
	std::vector<Tickable*> m_tickables;
	std::vector<Light*> m_lights;

public:

	// Base functions.
	void Tick(const FrameTime& time);

	// Specialized type lists.
	// ---------------------------------------------------------

	// Camera accessors.
	std::vector<Camera*>& Get_Cameras();
	void Add_Camera(Camera* camera);
	void Remove_Camera(Camera* camera);	
	
	// Light accessors.
	std::vector<Light*>& Get_Lights();
	void Add_Light(Light* light);
	void Remove_Light(Light* light);	

	// Generic scene type lists.
	// ---------------------------------------------------------

	// Drawable accessors.
	std::vector<Drawable*>& Get_Drawables();
	void Add_Drawable(Drawable* camera);
	void Remove_Drawable(Drawable* camera);	

	// Tickable accessors.
	std::vector<Tickable*>& Get_Tickables();
	void Add_Tickable(Tickable* camera);
	void Remove_Tickable(Tickable* camera);	

};

#endif

