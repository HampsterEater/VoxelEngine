// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef CONFIG_VALUE
#error "CONFIG_VALUE must be defined when including this file!"
#endif

// These define all the different configuration options stored in the GameEngineConfig
// class, this file is used to make serialization/deserialization cleaner and easier.

//			Type						Name								Serialized Location

// Tick settings.
CONFIG_VALUE(int,						target_frame_rate,					"rendering/frame_rate")

// Display settings.
CONFIG_VALUE(int,						display_width,						"rendering/display/width")
CONFIG_VALUE(int,						display_height,						"rendering/display/height")
CONFIG_VALUE(bool,						display_fullscreen,					"rendering/display/fullscreen")
CONFIG_VALUE(const char*,				display_title,						"rendering/display/title")

// Render pipeline settings.
CONFIG_VALUE(const char*,				render_pipeline_file,				"rendering/pipeline")

// Audio settings.
CONFIG_VALUE(int,						audio_max_channels,					"audio/max_channels")

// Language settings.
CONFIG_VALUE(const char*,				language_default,					"languages/default")
CONFIG_VALUE(std::vector<const char*>,	languages,							"languages/language")

// Task settings
CONFIG_VALUE(int,						tasks_max_workers,					"tasks/max_workers")
CONFIG_VALUE(int,						tasks_max_tasks,					"tasks/max_tasks")