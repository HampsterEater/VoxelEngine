// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_CONDITIONVARIABLE_
#define _GENERIC_CONDITIONVARIABLE_

class Mutex;

class ConditionVariable
{
public:
	static ConditionVariable* Create();

	// Lock / Unlock shenanigans.
	virtual void Broadcast	() = 0;
	virtual void Signal		() = 0;
	virtual bool Wait		(Mutex* mutex, unsigned int timeout = 0) = 0;

};

#endif

