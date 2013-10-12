// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_FMOD_FMODSOUNDCHANNEL_
#define _ENGINE_AUDIO_FMOD_FMODSOUNDCHANNEL_

#include "Engine\Audio\Sounds\SoundChannel.h"

#include "Engine\Audio\FMod\FMod_AudioRenderer.h"

class FMod_SoundChannel : public SoundChannel
{
private:
	friend class FMod_AudioRenderer;

private:
	FMod_AudioRenderer* m_renderer;
	FMOD_CHANNEL*		m_channel;

	float				m_default_frequency;
	float				m_default_volume;
	float				m_default_pan;
	int					m_default_priority;

public:

	// Destructor!
	FMod_SoundChannel(FMod_AudioRenderer* renderer, FMOD_CHANNEL* channel);
	~FMod_SoundChannel();

	// Sound channel stuff.
	void  Set_Volume(float volume);
	float Get_Volume();

	void  Set_Position(float volume);
	float Get_Position();

	void  Set_Pan(float pan);
	float Get_Pan();
	
	void  Set_Rate(float rate);
	float Get_Rate();

	bool Is_Paused();
	bool Is_Playing();
	bool Is_Muted();

	void Pause();
	void Resume();

	void Mute();
	void Unmute();

	// FMOD specific stuff.
	FMOD_CHANNEL* Get_FMod_Channel();
	void Set_FMod_Sound(FMOD_SOUND* sound);

};

#endif

