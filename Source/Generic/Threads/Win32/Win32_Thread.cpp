// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Threads\Win32\Win32_Thread.h"

// TODO: An ever growing, never deallocated heap of threads. God this is a bad
// idea, fix this shit soon. Dam Get_Current_Thread() >_>
std::vector<Win32_Thread*> Win32_Thread::g_win32_threads;

// This lovely struct + function are used to set the name of a thread
// in the visual studio debugger. Only works in visual studio!
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // must be 0x1000
   LPCSTR szName; // pointer to name (in user addr space)
   DWORD dwThreadID; // thread ID (-1=caller thread)
   DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

void Set_Visual_Studio_Thread_Name(DWORD dwThreadID, const char* szThreadName)
{
	THREADNAME_INFO info;
	info.dwType			= 0x1000;
	info.szName			= szThreadName;
	info.dwThreadID		= dwThreadID;
	info.dwFlags		= 0;

	__try
	{
		RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info);
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

Win32_Thread* Win32_Thread::Get_Current_Thread()
{
	HANDLE handle = GetCurrentThread();
	for (std::vector<Win32_Thread*>::iterator iter = g_win32_threads.begin(); iter != g_win32_threads.end(); iter++)
	{
		Win32_Thread* thread = *iter;
		if (thread->m_thread == handle)
		{
			return thread;
		}
	}

	Win32_Thread* thread = new Win32_Thread(handle);
	g_win32_threads.push_back(thread);

	return thread;
}

Win32_Thread::Win32_Thread(const char* name, EntryPoint entry_point, void* ptr) 
	: Thread(name) 
	, m_finished(false)
	, m_thread_id(0)
{
	m_entry_point = entry_point;
	m_entry_ptr = ptr;
	m_thread = CreateThread(NULL, NULL, Start_Routine, this, CREATE_SUSPENDED, &m_thread_id);

	DBG_ASSERT(m_thread != NULL);
}

Win32_Thread::Win32_Thread(HANDLE handle)
	: Thread(NULL)
	, m_finished(false)
	, m_thread_id(0)
{
	m_thread = handle;
	DBG_ASSERT(m_thread != NULL);
}

Win32_Thread::~Win32_Thread()
{
	// Wait for thread to terminate.
	while (!m_finished)
		Sleep(0.01f);

	// Close handle.
	int ret = CloseHandle(m_thread);
	DBG_ASSERT(ret != 0);
	m_thread = NULL;
}

DWORD WINAPI Win32_Thread::Start_Routine(LPVOID lpThreadParameter)
{
	Win32_Thread* thread = reinterpret_cast<Win32_Thread*>(lpThreadParameter);

	// Set thread name.
	if (thread->m_thread_id != 0 && thread->m_name != NULL)
	{
		Set_Visual_Studio_Thread_Name(thread->m_thread_id, thread->m_name);
	}

	// Call entry point.
	thread->m_entry_point(thread, thread->m_entry_ptr);
	thread->m_finished = true;

	return 0;
}

bool Win32_Thread::Is_Running()
{
	return !m_finished;
}

void Win32_Thread::Start()
{
	ResumeThread(m_thread);
}

void Win32_Thread::Set_Priority(ThreadPriority::Type priority)
{
	switch (priority)
	{
	case ThreadPriority::Highest:			SetThreadPriority(m_thread, THREAD_PRIORITY_HIGHEST);			return;	
	case ThreadPriority::High:				SetThreadPriority(m_thread, THREAD_PRIORITY_ABOVE_NORMAL);		return;	
	case ThreadPriority::Normal:			SetThreadPriority(m_thread, THREAD_PRIORITY_NORMAL);			return;	
	case ThreadPriority::Low:				SetThreadPriority(m_thread, THREAD_PRIORITY_BELOW_NORMAL);		return;	
	case ThreadPriority::Lowest:			SetThreadPriority(m_thread, THREAD_PRIORITY_LOWEST);			return;	
	default:
		DBG_ASSERT(false);
	}
}

void Win32_Thread::Set_Affinity(int mask)
{
	SetThreadAffinityMask(m_thread, mask);
}

void Win32_Thread::Sleep(float seconds)
{
	::Sleep((DWORD)(1000 * seconds));
}

int Win32_Thread::Get_Core_Count()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	return sysinfo.dwNumberOfProcessors;
}

void Win32_Thread::Yield()
{
	::Sleep(0);
}
