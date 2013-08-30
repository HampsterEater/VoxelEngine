// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_SEMAPHORE_
#define _GENERIC_SEMAPHORE_

class Semaphore
{
public:
	static Semaphore* Create();

	// Syncronization.
	virtual void Signal	() = 0;
	virtual void Wait	() = 0;

};

#endif

