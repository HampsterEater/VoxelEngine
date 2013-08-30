// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_ACTOR_
#define _ENGINE_ACTOR_

#include "Engine\Engine\FrameTime.h"

#include "Generic\Types\Vector3.h"

class Actor
{
protected:
	Vector3 m_position;
	Vector3 m_rotation;

public:

	// Constructors.
	Actor();

	// Base functions.
	virtual void Tick(const FrameTime& time) = 0;
	
	// Get modifiers.
	virtual Vector3 Get_Position();
	virtual void	Set_Position(Vector3 position);
	virtual Vector3 Get_Rotation();
	virtual void	Set_Rotation(Vector3 rotation);

};

#endif

