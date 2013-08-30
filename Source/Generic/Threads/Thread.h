// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_THREAD_
#define _GENERIC_THREAD_

struct ThreadPriority
{
	enum Type
	{
		Highest,
		High,
		Normal,
		Low,
		Lowest
	};
};

class Thread
{
public:
	typedef void (*EntryPoint)(Thread* self, void* ptr);
	static Thread* Create(EntryPoint entry_point, void* ptr);
	static Thread* Get_Current();

	// Control functions.
	virtual bool Is_Running() = 0;
	virtual void Start() = 0;
	virtual void Set_Priority(ThreadPriority::Type priority) = 0;
	virtual void Set_Affinity(int mask) = 0;

	// General functions.
	virtual void Sleep(float seconds) = 0;
	virtual int  Get_Core_Count() = 0;
	virtual void Yield() = 0;

};

#endif

