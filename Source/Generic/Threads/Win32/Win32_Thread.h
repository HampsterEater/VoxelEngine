// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_WIN32_THREAD_
#define _GENERIC_WIN32_THREAD_

#include "Generic\Threads\Thread.h"

#include <Windows.h>
#include <vector>

#ifdef PLATFORM_WIN32
#undef Yield // Fucking windows.
#endif

class Win32_Thread : public Thread
{
private:
	friend class Thread;

	Win32_Thread(HANDLE handle);
	Win32_Thread(const char* name, EntryPoint entry_point, void* ptr);

	static std::vector<Win32_Thread*> g_win32_threads;

	static Win32_Thread* Get_Current_Thread();

	static DWORD WINAPI Start_Routine(LPVOID lpThreadParameter);

	EntryPoint	m_entry_point;
	void*		m_entry_ptr;
	HANDLE		m_thread;
	bool		m_finished;
	DWORD		m_thread_id;

public:

	~Win32_Thread();

	// Control functions.
	void Start();
	void Set_Priority(ThreadPriority::Type priority);
	void Set_Affinity(int mask);

	// General functions.
	bool Is_Running();

	// Functions that should be static! TODO
	void Sleep(float seconds);
	int  Get_Core_Count();
	void Yield();

};

#endif

