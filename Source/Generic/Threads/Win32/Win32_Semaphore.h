// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_WIN32_SEMAPHORE_
#define _GENERIC_WIN32_SEMAPHORE_

#include "Generic\Threads\Semaphore.h"

#include <Windows.h>

class Win32_Semaphore : public Semaphore
{
private:
	friend class Semaphore;
	Win32_Semaphore();

	HANDLE m_handle;

public:

	~Win32_Semaphore();

	void Signal	();
	void Wait	();

};

#endif

