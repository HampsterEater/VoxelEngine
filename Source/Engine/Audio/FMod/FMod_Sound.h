// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_FMOD_FMODSOUND_
#define _ENGINE_AUDIO_FMOD_FMODSOUND_

#include "Engine\Audio\Sounds\Sound.h"
#include "Generic\ThirdParty\FMod\fmod.hpp"

class FMod_AudioRenderer;

class FMod_Sound : public Sound
{
private:
	friend class FMod_AudioRenderer;

private:
	FMOD_SOUND* m_sound;
	FMod_AudioRenderer* m_renderer;

public:

	// Destructor!
	FMod_Sound(FMod_AudioRenderer* renderer, FMOD_SOUND* sound);
	~FMod_Sound();

	// Actual sound stuff!
	void Play(SoundChannel* channel = NULL, bool start_paused = false) const;
	SoundChannel* Allocate_Channel() const;

};

#endif

