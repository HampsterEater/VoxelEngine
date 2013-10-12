// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Audio\FMod\FMod_SoundChannel.h"

FMOD_CHANNEL* FMod_SoundChannel::Get_FMod_Channel()
{
	return m_channel;
}

void FMod_SoundChannel::Set_FMod_Sound(FMOD_SOUND* sound)
{
	FMOD_RESULT result = FMOD_Sound_GetDefaults(sound, &m_default_frequency, &m_default_volume, &m_default_pan, &m_default_priority);
	DBG_ASSERT(result == FMOD_OK);
}

FMod_SoundChannel::FMod_SoundChannel(FMod_AudioRenderer* renderer, FMOD_CHANNEL* channel)
	: m_renderer(renderer)
	, m_channel(channel)
{
}

FMod_SoundChannel::~FMod_SoundChannel()
{
	// Release ref-counted fmod channel reference!
	m_channel = NULL;
}

void FMod_SoundChannel::Set_Volume(float volume)
{
	FMOD_RESULT result = FMOD_Channel_SetVolume(m_channel, volume);
	DBG_ASSERT(result == FMOD_OK);
}

float FMod_SoundChannel::Get_Volume()
{
	float volume = 0.0f;

	FMOD_RESULT result = FMOD_Channel_GetVolume(m_channel, &volume);
	DBG_ASSERT(result == FMOD_OK);

	return volume;
}

void FMod_SoundChannel::Set_Position(float position)
{
	FMOD_RESULT result = FMOD_Channel_SetPosition(m_channel, (unsigned int)position, FMOD_TIMEUNIT_MS);
	DBG_ASSERT(result == FMOD_OK);
}

float FMod_SoundChannel::Get_Position()
{	
	unsigned int position = 0;

	FMOD_RESULT result = FMOD_Channel_GetPosition(m_channel, &position, FMOD_TIMEUNIT_MS);
	DBG_ASSERT(result == FMOD_OK);

	return (float)position;
}

void FMod_SoundChannel::Set_Pan(float pan)
{
	FMOD_RESULT result = FMOD_Channel_SetPan(m_channel, pan);
	DBG_ASSERT(result == FMOD_OK);
}

float FMod_SoundChannel::Get_Pan()
{
	float pan = 0.0f;

	FMOD_RESULT result = FMOD_Channel_GetPan(m_channel, &pan);
	DBG_ASSERT(result == FMOD_OK);

	return pan;
}
	
void FMod_SoundChannel::Set_Rate(float rate)
{
	FMOD_RESULT result = FMOD_Channel_SetFrequency(m_channel, m_default_frequency * rate);
	DBG_ASSERT(result == FMOD_OK);
}

float FMod_SoundChannel::Get_Rate()
{
	float frequency = 0.0f;

	FMOD_RESULT result = FMOD_Channel_GetFrequency(m_channel, &frequency);
	DBG_ASSERT(result == FMOD_OK);

	return frequency / m_default_frequency;
}

bool FMod_SoundChannel::Is_Paused()
{
	FMOD_BOOL paused = false;

	FMOD_RESULT result = FMOD_Channel_GetPaused(m_channel, &paused);
	DBG_ASSERT(result == FMOD_OK);

	return paused;
}

bool FMod_SoundChannel::Is_Playing()
{
	FMOD_BOOL playing = false;

	FMOD_RESULT result = FMOD_Channel_IsPlaying(m_channel, &playing);
	DBG_ASSERT(result == FMOD_OK);

	return playing;
}

bool FMod_SoundChannel::Is_Muted()
{
	FMOD_BOOL mute = false;

	FMOD_RESULT result = FMOD_Channel_GetMute(m_channel, &mute);
	DBG_ASSERT(result == FMOD_OK);

	return mute;
}

void FMod_SoundChannel::Pause()
{
	FMOD_RESULT result = FMOD_Channel_SetPaused(m_channel, true);
	DBG_ASSERT(result == FMOD_OK);
}

void FMod_SoundChannel::Resume()
{
	FMOD_RESULT result = FMOD_Channel_SetPaused(m_channel, false);
	DBG_ASSERT(result == FMOD_OK);
}

void FMod_SoundChannel::Mute()
{
	FMOD_RESULT result = FMOD_Channel_SetMute(m_channel, true);
	DBG_ASSERT(result == FMOD_OK);
}

void FMod_SoundChannel::Unmute()
{
	FMOD_RESULT result = FMOD_Channel_SetMute(m_channel, false);
	DBG_ASSERT(result == FMOD_OK);
}

