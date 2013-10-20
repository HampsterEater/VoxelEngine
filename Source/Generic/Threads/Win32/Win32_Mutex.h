// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_WIN32_MUTEX_
#define _GENERIC_WIN32_MUTEX_

#include "Generic\Threads\Mutex.h"

#include <Windows.h>

class Win32_ConditionVariable;

class Win32_Mutex : public Mutex
{
private:
	friend class Mutex;
	friend class Win32_ConditionVariable;

	Win32_Mutex();

	CRITICAL_SECTION m_critical_section;

public:

	~Win32_Mutex();
	
	bool Try_Lock	();
	void Lock		();
	void Unlock		();

};

#endif

