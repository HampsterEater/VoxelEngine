// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_WIN32_CONDITIONVARIABLE_
#define _GENERIC_WIN32_CONDITIONVARIABLE_

#include "Generic\Threads\ConditionVariable.h"

#include <Windows.h>

class Win32_ConditionVariable : public ConditionVariable
{
private:
	friend class ConditionVariable;
	Win32_ConditionVariable();
		
	CONDITION_VARIABLE m_convar_handle;
		
public:
	~Win32_ConditionVariable();
	
	void Broadcast();
	void Signal();
	bool Wait(Mutex* mutex, unsigned int timeout);

};

#endif

