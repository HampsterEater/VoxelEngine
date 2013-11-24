// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_TICKABLE_
#define _ENGINE_TICKABLE_

#include "Engine\Engine\FrameTime.h"

#include "Generic\Types\Vector3.h"

class Tickable
{
protected:
	bool m_enabled;

public:

	Tickable();

	// Base functions.
	virtual void Tick(const FrameTime& time) = 0;

	// State changes.
	virtual void Set_Enabled(bool value);
	virtual bool Get_Enabled() const;

};

#endif

