// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Textures\Texture.h"

LinkedList<TextureFactory*> TextureFactory::m_factories;

Texture* TextureFactory::Load(const char* url, TextureFlags::Type flags)
{
	for (LinkedList<TextureFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		TextureFactory* factory = *iter;
		Texture* result = factory->Try_Load(url, flags);
		if (result != NULL)
		{
			DBG_LOG("Loaded texture: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to load texture: %s", url);
	return NULL;
}

TextureFactory::TextureFactory()
{
	m_factories.Add(this);
}

TextureFactory::~TextureFactory()
{
	m_factories.Remove(m_factories.Find(this));
}
