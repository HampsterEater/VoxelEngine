// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_FMOD_FMODSOUNDFACTORY_
#define _ENGINE_AUDIO_FMOD_FMODSOUNDFACTORY_

#include "Engine\Audio\Sounds\SoundFactory.h"

class FMod_SoundFactory : public SoundFactory
{
private:

public:
	Sound* Try_Load(const char* url, SoundFlags::Type flags);

};

#endif

