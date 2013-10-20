// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Threads\Win32\Win32_ConditionVariable.h"
#include "Generic\Threads\Win32\Win32_Mutex.h"

Win32_ConditionVariable::Win32_ConditionVariable()
{
	InitializeConditionVariable(&m_convar_handle);
}

Win32_ConditionVariable::~Win32_ConditionVariable()
{
	// Nothing to delete in win32.
}

void Win32_ConditionVariable::Broadcast()
{
	WakeAllConditionVariable(&m_convar_handle);
}

void Win32_ConditionVariable::Signal()
{
	WakeConditionVariable(&m_convar_handle);
}

bool Win32_ConditionVariable::Wait(Mutex* mutex, unsigned int timeout)
{
	Win32_Mutex* win32mutex = static_cast<Win32_Mutex*>(mutex);

	if (timeout == 0)
	{
		timeout = INFINITE;
	}

	return (SleepConditionVariableCS(&m_convar_handle, &win32mutex->m_critical_section, timeout) != 0);
}
