// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_KEYS_
#define _ENGINE_KEYS_

struct Key
{
	enum Type
	{
		Up,
		Down,
		Left,
		Right,

		Escape,
		F1
	};
};

#endif