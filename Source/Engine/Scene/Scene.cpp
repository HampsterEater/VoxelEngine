// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Scene\Scene.h"
#include "Engine\Scene\Camera.h"
#include "Engine\Scene\Light.h"
#include "Engine\Scene\Tickable.h"

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

std::vector<Light*>& Scene::Get_Lights()
{
	return m_lights;
}

void Scene::Add_Light(Light* camera)
{
	if (std::find(m_lights.begin(), m_lights.end(), camera) == m_lights.end())
	{
		m_lights.push_back(camera);
	}
}

void Scene::Remove_Light(Light* camera)
{
	std::vector<Light*>::iterator iter = std::find(m_lights.begin(), m_lights.end(), camera);
	if (iter != m_lights.end())
	{
		m_lights.erase(iter);
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

std::vector<Tickable*>& Scene::Get_Tickables()
{
	return m_tickables;
}

void Scene::Add_Tickable(Tickable* camera)
{
	if (std::find(m_tickables.begin(), m_tickables.end(), camera) == m_tickables.end())
	{
		m_tickables.push_back(camera);
	}
}

void Scene::Remove_Tickable(Tickable* camera)
{
	std::vector<Tickable*>::iterator iter = std::find(m_tickables.begin(), m_tickables.end(), camera);
	if (iter != m_tickables.end())
	{
		m_tickables.erase(iter);
	}
}

void Scene::Tick(const FrameTime& time)
{
	// Tick all tickables.
	for (std::vector<Tickable*>::iterator iter = m_tickables.begin(); iter != m_tickables.end(); iter++)
	{
		Tickable* tickable = *iter;

		if (tickable->Get_Enabled())
		{
			tickable->Tick(time);
		}
	}
}

