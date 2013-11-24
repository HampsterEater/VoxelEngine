// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_SOUNDS_SOUNDFACTORY_
#define _ENGINE_AUDIO_SOUNDS_SOUNDFACTORY_

#include "Generic\Types\LinkedList.h"
#include "Generic\Types\HashTable.h"
#include "Engine\IO\Stream.h"

#include "Engine\Audio\Sounds\Sound.h"
#include "Engine\Audio\Sounds\SoundHandle.h"

class SoundFactory
{
private:
	static LinkedList<SoundFactory*>	m_factories;
	static HashTable<SoundHandle*, int>	m_loaded_sounds;

public:
	
	// Dispose.
	static void Dispose();
	
	// Static methods.
	static SoundHandle* Load(const char* url, SoundFlags::Type flags);
	static Sound* Load_Without_Handle(const char* url, SoundFlags::Type flags);

	// Constructors
	SoundFactory();	
	virtual ~SoundFactory();	

	// Derived factory methods.
	virtual Sound* Try_Load(const char* url, SoundFlags::Type flags) = 0;

};

#endif

