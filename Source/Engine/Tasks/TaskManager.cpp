// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Tasks\TaskManager.h"
#include "Generic\Threads\MutexLock.h"
#include "Generic\Threads\Thread.h"
#include "Generic\Math\Math.h"

void TaskManager::QueuedTask::Reset()
{
	ID				= -1;
	Work			= NULL;
	TaskRemaining	= 0;
	Parent			= 0;
	Dependency		= -1;
}

void TaskManager::Worker_Thread_Entry_Point(Thread* self, void* meta)
{
	TaskManager* manager = (TaskManager*)meta;

	while (!manager->m_workers_exiting)
	{
		Run_Available_Task(manager, 0);
	}
}

void TaskManager::Run_Available_Task(TaskManager* manager, unsigned int timeout)
{
	QueuedTask* task = manager->Worker_Wait_For_Task(timeout);
	if (task != NULL)
	{
		try
		{
			task->Work->Run();
		}
		catch (...)
		{
			DBG_ASSERT_STR(false, "Captured unhandled exception within worker thread.");
		}
		manager->Worker_Task_Completed(task);
	}
}

TaskManager::QueuedTask* TaskManager::Get_Task_By_ID(TaskID id)
{
	if (id == -1)
		return NULL;

	for (unsigned int i = 0; i < m_task_count; i++)
	{
		if (m_tasks[i].ID == id)
		{
			return &m_tasks[i];
		}
	}
	return NULL;
}

TaskManager::TaskManager(int max_workers, int max_tasks)
{
	m_worker_threads		 = new Thread*[max_workers];
	m_tasks					 = new QueuedTask[max_tasks];
	m_worker_count			 = max_workers;
	m_task_count			 = max_tasks;

	m_workers_exiting		 = false;
	m_next_task_id			 = 0;

	m_worker_task_con_var    = ConditionVariable::Create();
	m_worker_task_list_mutex = Mutex::Create();
	m_worker_task_mutex      = Mutex::Create();
	
	DBG_LOG("Max tasks: %i", max_tasks);
	DBG_LOG("Max task workers: %i", max_workers);
	
	for (unsigned int  i = 0; i < m_task_count; i++)
	{
		m_tasks[i].Reset();
	}
	for (unsigned int i = 0; i < m_worker_count; i++)
	{
		m_worker_threads[i] = Thread::Create("Task Worker", Worker_Thread_Entry_Point, this);

		int cores = m_worker_threads[i]->Get_Core_Count();		
		if (cores > 1)
		{
			int core = 1 + (i % (cores - 1));
			m_worker_threads[i]->Set_Affinity(1 << core);
			DBG_LOG("Starting task worker thread %i on core %i.", i, core);
		}
		else
		{
			DBG_LOG("Starting task worker thread %i on primary core.");
		}

		m_worker_threads[i]->Start();
	}
}

TaskManager::~TaskManager()
{	
	for (unsigned int i = 0; i < m_worker_count; i++)
	{
		SAFE_DELETE(m_worker_threads[i]);
	}

	SAFE_DELETE(m_worker_threads);
	SAFE_DELETE(m_tasks);

	SAFE_DELETE(m_worker_task_con_var);
	SAFE_DELETE(m_worker_task_list_mutex);
	SAFE_DELETE(m_worker_task_mutex);
}

TaskManager::QueuedTask* TaskManager::Worker_Wait_For_Task(unsigned int timeout)
{
	QueuedTask* task = Get_Available_Task();
	if (task != NULL)
		return task;

	{
		MutexLock lock(m_worker_task_mutex);

		if (m_worker_task_con_var->Wait(m_worker_task_mutex, timeout))
		{
			task = Get_Available_Task();
		}
	}

	return task;
}

void TaskManager::Worker_Post_New_Task(QueuedTask* task)
{
	{
		MutexLock lock(m_worker_task_list_mutex);
		m_task_queue.push_back(task);
	}
	{
		MutexLock lock(m_worker_task_mutex);
		m_worker_task_con_var->Broadcast();
	}
}

void TaskManager::Worker_Task_Completed(QueuedTask* task)
{
	task->TaskRemaining--;
	task->ID = -1;
	
	QueuedTask* parent = Get_Task_By_ID(task->Parent);
	if (parent != NULL)
		parent->TaskRemaining--;
	
	{
		MutexLock lock(m_worker_task_mutex);
		m_worker_task_con_var->Broadcast();
	}
}

TaskManager::QueuedTask* TaskManager::Get_Available_Task()
{
	QueuedTask* task = NULL;

	if (m_workers_exiting == true)
		return task;

	{
		MutexLock lock(m_worker_task_list_mutex);

		for (std::vector<QueuedTask*>::iterator iter = m_task_queue.begin(); iter != m_task_queue.end(); iter++)
		{
			QueuedTask* iter_task = *iter;
			if (iter_task->TaskRemaining == 1 && 
				Get_Task_By_ID(iter_task->Dependency) == NULL)
			{
				iter = m_task_queue.erase(iter);
				task = iter_task;
				continue;
			}
		}
	}

	return task;
}

TaskID TaskManager::Add_Task(Task* work, TaskID parent)
{
	QueuedTask* taskParent = Get_Task_By_ID(parent);
	if (taskParent != NULL)
	{
		taskParent->TaskRemaining++;
	}

	for (int i = 0; i < m_task_count; i++)
	{
		if (m_tasks[i].TaskRemaining <= 0)
		{
			m_tasks[i].Reset();
			m_tasks[i].ID			= (m_next_task_id++) % m_task_count;
			m_tasks[i].Parent		= parent;
			m_tasks[i].TaskRemaining = 1;
			m_tasks[i].Work			= work;

			return m_tasks[i].ID;
		}
	}

	DBG_ASSERT_STR(false, "Failed to add task into task queue, queue has overflowed!");
	return 0;
}

void TaskManager::Depends_On(TaskID work, TaskID on)
{
	QueuedTask* task = Get_Task_By_ID(work);
	if (task != NULL)
	{
		task->Dependency = on;
	}
}

void TaskManager::Wait_For(TaskID work)
{
	while (true)
	{
		// Task completed yet?
		QueuedTask* task = Get_Task_By_ID(work);
		if (task == NULL)
			return;

		// Run some tasks while we are waiting.	
		Run_Available_Task(this, 1);
	}
}

void TaskManager::Queue_Task(TaskID work)
{
	QueuedTask* task = Get_Task_By_ID(work);
	if (task != NULL)
	{
		Worker_Post_New_Task(task);
	}
}

void TaskManager::Wait_For_All()
{
	while (true)
	{
		// Task completed yet?
		bool found_active_task = true;
		for (int i = 0; i < m_task_count; i++)
		{
			if (m_tasks[i].TaskRemaining > 0)
			{
				found_active_task = true;
				break;
 			}
		}
		if (found_active_task == false)
		{
			break;
		}

		// Run some tasks while we are waiting.
		Run_Available_Task(this, 1);
	}
}

