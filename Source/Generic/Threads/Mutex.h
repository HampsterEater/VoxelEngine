// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MUTEX_
#define _GENERIC_MUTEX_

class Mutex
{
public:
	static Mutex* Create();

	// Lock / Unlock shenanigans.
	virtual bool Try_Lock	() = 0;
	virtual void Lock		() = 0;
	virtual void Unlock		() = 0;

};

#endif

