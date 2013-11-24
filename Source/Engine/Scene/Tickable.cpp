// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Scene\Tickable.h"

Tickable::Tickable()
	: m_enabled(true)
{
}

void Tickable::Set_Enabled(bool value)
{
	m_enabled = value;
}

bool Tickable::Get_Enabled() const
{
	return m_enabled;
}