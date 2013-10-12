// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Audio\AudioRenderer.h"
#include "Engine\Audio\FMod\FMod_AudioRenderer.h"
#include "Engine\Audio\FMod\FMod_SoundFactory.h"
#include "Engine\Audio\FMod\FMod_Sound.h"

Sound* FMod_SoundFactory::Try_Load(const char* url, SoundFlags::Type flags)
{
	FMod_AudioRenderer* renderer = dynamic_cast<FMod_AudioRenderer*>(AudioRenderer::Get());
	DBG_ASSERT(renderer != NULL);

	FMOD_SYSTEM* system = renderer->Get_FMod_System();
	FMOD_MODE mode = FMOD_DEFAULT | FMOD_CREATESAMPLE;

	if ((flags & SoundFlags::Loop) != 0)
	{
		mode = FMOD_HARDWARE | FMOD_LOOP_NORMAL | FMOD_CREATESAMPLE;
	}

	FMOD_SOUND* sound = NULL;
	FMOD_RESULT result = FMOD_System_CreateSound(system, url, mode, NULL, &sound);
	if (result != FMOD_OK)
	{
		DBG_LOG("FMod failed to load sound from '%s' due to error 0x%08x", url, result);
		return NULL;
	}

	return new FMod_Sound(renderer, sound);
}