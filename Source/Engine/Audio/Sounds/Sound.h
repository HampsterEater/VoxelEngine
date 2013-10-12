// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_AUDIO_SOUNDS_SOUND_
#define _ENGINE_AUDIO_SOUNDS_SOUND_

class SoundChannel;

struct SoundFlags
{
	enum Type
	{
		None = 0,
		Loop = 1
	};
};

class Sound
{
private:
	
protected:

public:

	// Destructor!
	virtual ~Sound();

	// Actual sound stuff!
	virtual void		  Play				(SoundChannel* channel = NULL, bool start_paused = false) const = 0;
	virtual SoundChannel* Allocate_Channel	() const = 0;

};

#endif

