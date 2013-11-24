// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Textures\PixmapFactory.h"
#include "Engine\Renderer\Textures\Pixmap.h"

#include "Generic\Helper\StringHelper.h"

LinkedList<PixmapFactory*> PixmapFactory::m_factories;

bool PixmapFactory::Save(const char* url, Pixmap* texture)
{
	for (LinkedList<PixmapFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		PixmapFactory* factory = *iter;
		bool result = factory->Try_Save(url, texture);
		if (result != false)
		{
			DBG_LOG("Saved pixmap: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to save pixmap: %s", url);
	return NULL;
}

Pixmap* PixmapFactory::Load(const char* url)
{
	for (LinkedList<PixmapFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		PixmapFactory* factory = *iter;
		Pixmap* result = factory->Try_Load(url);
		if (result != NULL)
		{
			DBG_LOG("Loaded pixmap: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to load pixmap: %s", url);
	return NULL;
}

PixmapFactory::PixmapFactory()
{
	m_factories.Add(this);
}

PixmapFactory::~PixmapFactory()
{
	m_factories.Remove(m_factories.Find(this));
}
