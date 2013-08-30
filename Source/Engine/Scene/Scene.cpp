// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Scene\Scene.h"
#include "Engine\Scene\Camera.h"

#include <algorithm>

std::vector<Camera*>& Scene::Get_Cameras()
{
	return m_cameras;
}

void Scene::Add_Camera(Camera* camera)
{
	if (std::find(m_cameras.begin(), m_cameras.end(), camera) == m_cameras.end())
	{
		m_cameras.push_back(camera);
	}
}

void Scene::Remove_Camera(Camera* camera)
{
	std::vector<Camera*>::iterator iter = std::find(m_cameras.begin(), m_cameras.end(), camera);
	if (iter != m_cameras.end())
	{
		m_cameras.erase(iter);
	}
}

std::vector<Drawable*>& Scene::Get_Drawables()
{
	return m_drawables;
}

void Scene::Add_Drawable(Drawable* camera)
{
	if (std::find(m_drawables.begin(), m_drawables.end(), camera) == m_drawables.end())
	{
		m_drawables.push_back(camera);
	}
}

void Scene::Remove_Drawable(Drawable* camera)
{
	std::vector<Drawable*>::iterator iter = std::find(m_drawables.begin(), m_drawables.end(), camera);
	if (iter != m_drawables.end())
	{
		m_drawables.erase(iter);
	}
}

void Scene::Tick(const FrameTime& time)
{
	// Tick all cameras.
	for (std::vector<Camera*>::iterator iter = m_cameras.begin(); iter != m_cameras.end(); iter++)
	{
		(*iter)->Tick(time);
	}
}

