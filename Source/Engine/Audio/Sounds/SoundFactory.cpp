// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Audio\Sounds\SoundFactory.h"
#include "Engine\Audio\Sounds\Sound.h"

#include "Generic\Helper\StringHelper.h"

LinkedList<SoundFactory*>		SoundFactory::m_factories;
HashTable<SoundHandle*, int>	SoundFactory::m_loaded_sounds;

SoundHandle* SoundFactory::Load(const char* url, SoundFlags::Type flags)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_sounds.Contains(url_hash))
	{
		DBG_LOG("Loaded sound from cache: %s", url);
		return m_loaded_sounds.Get(url_hash);
	}

	// Try and load texture!
	Sound* sound = Load_Without_Handle(url, flags);
	if (sound != NULL)
	{		
		SoundHandle* handle = new SoundHandle(url, flags, sound);

		m_loaded_sounds.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

Sound* SoundFactory::Load_Without_Handle(const char* url, SoundFlags::Type flags)
{
	for (LinkedList<SoundFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		SoundFactory* factory = *iter;
		Sound* result = factory->Try_Load(url, flags);
		if (result != NULL)
		{
			DBG_LOG("Loaded sound: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to load sound: %s", url);
	return NULL;
}

SoundFactory::SoundFactory()
{
	m_factories.Add(this);
}

SoundFactory::~SoundFactory()
{
	m_factories.Remove(m_factories.Find(this));
}
