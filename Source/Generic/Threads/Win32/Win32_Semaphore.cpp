// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Threads\Win32\Win32_Semaphore.h"

Win32_Semaphore::Win32_Semaphore()
{
	m_handle = CreateSemaphore(NULL, 0, 1000000, NULL);
	DBG_ASSERT(m_handle != NULL);
}

Win32_Semaphore::~Win32_Semaphore()
{
	CloseHandle(m_handle);
}

void Win32_Semaphore::Signal()
{
	ReleaseSemaphore(m_handle, 1, NULL);
}

void Win32_Semaphore::Wait()
{
	WaitForSingleObject(m_handle, INFINITE);
}
