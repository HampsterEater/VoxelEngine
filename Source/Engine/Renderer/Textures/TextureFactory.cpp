// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Textures\Texture.h"

#include "Generic\Helper\StringHelper.h"

LinkedList<TextureFactory*>		TextureFactory::m_factories;
HashTable<TextureHandle*, int>	TextureFactory::m_loaded_textures;

bool TextureFactory::Save(const char* url, Texture* texture, TextureFlags::Type flags)
{
	for (LinkedList<TextureFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		TextureFactory* factory = *iter;
		bool result = factory->Try_Save(url, texture, flags);
		if (result != false)
		{
			DBG_LOG("Saved texture: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to save texture: %s", url);
	return NULL;
}

TextureHandle* TextureFactory::Load(const char* url, TextureFlags::Type flags)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_textures.Contains(url_hash))
	{
		DBG_LOG("Loaded texture from cache: %s", url);
		return m_loaded_textures.Get(url_hash);
	}

	// Try and load texture!
	Texture* texture = Load_Without_Handle(url, flags);
	if (texture != NULL)
	{		
		TextureHandle* handle = new TextureHandle(url, flags, texture);

		m_loaded_textures.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

Texture* TextureFactory::Load_Without_Handle(const char* url, TextureFlags::Type flags)
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
	for (HashTable<TextureHandle*, int>::Iterator iter = m_loaded_textures.Begin(); iter != m_loaded_textures.End(); iter++)
	{
		TextureHandle* handle = *iter;
		SAFE_DELETE(handle);
	}

	m_loaded_textures.Clear();
	m_factories.Remove(m_factories.Find(this));
}
