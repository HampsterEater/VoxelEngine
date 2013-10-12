// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_FMOD_AUDIORENDERER_
#define _ENGINE_AUDIO_FMOD_AUDIORENDERER_

#include "Generic\Patterns\Singleton.h"
#include "Generic\Types\Matrix4.h"
#include "Engine\Audio\AudioRenderer.h"

#include "Generic\ThirdParty\FMod\fmod.hpp"

#include <vector>

class FMod_Sound;

// FMod rendering fun times!
class FMod_AudioRenderer : public AudioRenderer
{
private:
	friend class AudioRenderer;

	FMOD_SYSTEM* m_system;

	static FMOD_RESULT F_CALLBACK fmod_open (const char* name, int unicode, unsigned int* filesize, void** handle, void** userdata);
	static FMOD_RESULT F_CALLBACK fmod_close(void* handle, void* userdata);
	static FMOD_RESULT F_CALLBACK fmod_seek (void* handle, unsigned int pos, void* userdata);
	static FMOD_RESULT F_CALLBACK fmod_read (void* handle, void* buffer, unsigned int sizebytes, unsigned int* bytesread, void* userdata);

	FMod_AudioRenderer();
	~FMod_AudioRenderer();

public:

	// FMOD Specific.
	FMOD_SYSTEM* Get_FMod_System();
	
	// Base functions.	
	void Tick(const FrameTime& time);

};

#endif

