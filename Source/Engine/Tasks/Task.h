// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_TASKS_TASK_
#define _ENGINE_TASKS_TASK_

// Used in conjunction with the task manager to spin up and run
// a multi-threaded task.
class Task
{
private:
	bool m_completed;

protected:
	friend class TaskManager;

	Task();
	void Reset();
	void Complete();

public:	
	bool Is_Completed();

	virtual void Run() = 0;

};

#endif

