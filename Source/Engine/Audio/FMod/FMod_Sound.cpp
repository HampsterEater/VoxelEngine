// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Audio\FMod\FMod_AudioRenderer.h"
#include "Engine\Audio\FMod\FMod_Sound.h"
#include "Engine\Audio\FMod\FMod_SoundChannel.h"

FMod_Sound::FMod_Sound(FMod_AudioRenderer* renderer, FMOD_SOUND* sound)
	: m_sound(sound)
	, m_renderer(renderer)
{
}

FMod_Sound::~FMod_Sound()
{
	FMOD_Sound_Release(m_sound);
	m_sound = NULL;
}

void FMod_Sound::Play(SoundChannel* channel, bool start_paused) const
{
	FMod_SoundChannel*	fmod_channel = dynamic_cast<FMod_SoundChannel*>(channel);
	FMOD_SYSTEM*		system		 = m_renderer->Get_FMod_System();
	FMOD_CHANNEL*		chan		 = fmod_channel != NULL ? fmod_channel->Get_FMod_Channel() : NULL;

	FMOD_RESULT result = FMOD_System_PlaySound(system, channel != NULL ? FMOD_CHANNEL_REUSE : FMOD_CHANNEL_FREE, m_sound, start_paused, chan != NULL ? &chan : NULL);
	if (result != FMOD_OK)
	{
		DBG_LOG("Play fmod sound failed with error %i\n", result);
	}

	// Update channel so it knows its playing this sound.
	if (fmod_channel != NULL)
	{
		fmod_channel->Set_FMod_Sound(m_sound);
	}
}

SoundChannel* FMod_Sound::Allocate_Channel() const
{
	FMOD_SYSTEM*  system = m_renderer->Get_FMod_System();
	FMOD_CHANNEL* chan	 = NULL;

	FMOD_RESULT result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, m_sound, true, &chan);
	if (result != FMOD_OK)
	{
		DBG_LOG("Allocate fmod channel failed with error %i\n", result);
		return NULL;
	}

	// Prevent fmod stealing this channel.
	result = FMOD_Channel_SetPriority(chan, 0);
	if (result != FMOD_OK)
	{
		DBG_LOG("Failed to set fmod channel priority with error %i\n", result);
	}

	// Create actual fmod channel.
	FMod_SoundChannel* fmod_channel = new FMod_SoundChannel(m_renderer, chan);
	fmod_channel->Set_FMod_Sound(m_sound);

	// Return channel instance.
	return fmod_channel;
}
