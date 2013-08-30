// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Scene\Actor.h"

Actor::Actor()
	: m_position(0, 0, 0)
	, m_rotation(0, 0, 0)
{
}

Vector3 Actor::Get_Position()
{
	return m_position;
}

void Actor::Set_Position(Vector3 position)
{
	m_position = position;
}

Vector3 Actor::Get_Rotation()
{
	return m_rotation;
}

void Actor::Set_Rotation(Vector3 rotation)
{
	m_rotation = rotation;
}