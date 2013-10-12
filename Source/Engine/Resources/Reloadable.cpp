// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Resources\Reloadable.h"

std::vector<Reloadable*> Reloadable::m_reloadables;

void Reloadable::Add_Reload_Trigger_File(const char* path)
{
	m_watchers.push_back(new FileWatcher(path));
}

void Reloadable::Reset_Reload_Trigger_Files()
{
	for (std::vector<FileWatcher*>::iterator iter = m_watchers.begin(); iter != m_watchers.end(); iter++)
	{
		delete *iter;
	}
	m_watchers.clear();
}

void Reloadable::Check_For_Reloads()
{
	for (std::vector<Reloadable*>::iterator iter = m_reloadables.begin(); iter != m_reloadables.end(); iter++)
	{
		Reloadable* reloadable = *iter;
		bool changed = false;

		for (std::vector<FileWatcher*>::iterator iter = reloadable->m_watchers.begin(); iter != reloadable->m_watchers.end(); iter++)
		{
			if ((*iter)->Has_Changed() == true)
			{
				changed = true;
				// Note: Do not break out of this loop, we need to clear the has-changed flag for all of them.
			}
		}

		if (changed == true)
		{
			(*iter)->Reload();
		}
	}
}

Reloadable::Reloadable()
{
	m_reloadables.push_back(this);
}

Reloadable::~Reloadable()
{
	for (std::vector<FileWatcher*>::iterator iter = m_watchers.begin(); iter != m_watchers.end(); iter++)
	{
		delete *iter;
	}
	m_watchers.clear();
	m_reloadables.erase(std::find(m_reloadables.begin(), m_reloadables.end(), this));
}
