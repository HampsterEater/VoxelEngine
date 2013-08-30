// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_GAMEENGINECONFIG_
#define _ENGINE_GAMEENGINECONFIG_

class GameEngineConfig
{
public:

	// Tick settings.
	int				target_frame_rate;

	// Display settings.
	int				display_width;
	int				display_height;
	bool			display_fullscreen;
	const char*		display_title;

};

#endif