// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Threads\MutexLock.h"
#include "Generic\Threads\Mutex.h"

MutexLock::MutexLock(Mutex* mutex)
{
	m_mutex = mutex;
	m_mutex->Lock();
}

MutexLock::~MutexLock()
{
	m_mutex->Unlock();
	m_mutex = NULL;
}
