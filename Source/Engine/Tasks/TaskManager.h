// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_TASKS_TASKMANAGER_
#define _ENGINE_TASKS_TASKMANAGER_

#include "Engine\Tasks\Task.h"
#include "Generic\Patterns\Singleton.h"

#include <vector>
#include "Generic\Threads\Thread.h"
#include "Generic\Threads\Mutex.h"
#include "Generic\Threads\ConditionVariable.h"

// A task ID is used to reference a task once it's been queued with the
// task amanager. It pretty much just equates to an index in the task array.
typedef unsigned int TaskID;

class TaskManager : public Singleton<TaskManager>
{
private:
	struct QueuedTask
	{
		TaskID			ID;
		Task*			Work;
		unsigned int	TaskRemaining;
		TaskID			Parent;
		TaskID			Dependency;	

		void Reset();
	};

private:
	// Worker information variables.
	TaskID		 m_next_task_id;
	QueuedTask*  m_tasks;
	int			 m_task_count;

	std::vector<QueuedTask*> m_task_queue;

	ConditionVariable*	m_worker_task_con_var;
	Mutex*				m_worker_task_list_mutex;
	Mutex*				m_worker_task_mutex;
	Thread**			m_worker_threads;
	int					m_worker_count;

	bool				m_workers_exiting;

	// Entry point for work threads.
	static void Worker_Thread_Entry_Point(Thread* self, void* meta);
	static void Run_Available_Task(TaskManager* manager, unsigned int timeout);

	// Used internally by workers to syncronize themselves.
	QueuedTask*	Worker_Wait_For_Task	(unsigned int timeout = 0);
	void		Worker_Post_New_Task	(QueuedTask* task);
	void		Worker_Task_Completed	(QueuedTask* task);

	QueuedTask*	Get_Available_Task		();
	QueuedTask*	Get_Task_By_ID			(TaskID id);

public:

	// Constructor/Destructor stuff.
	TaskManager							(int max_workers, int max_tasks);
	~TaskManager						();

	// Task management.
	TaskID Add_Task						(Task* work, TaskID parent = -1);
	void   Depends_On					(TaskID work, TaskID on);
	void   Wait_For						(TaskID work);
	void   Queue_Task					(TaskID work);
	void   Wait_For_All					();

};

#endif

