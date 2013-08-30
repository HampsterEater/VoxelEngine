// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Math\Random.h"

Random::Random(int seed)
{
	m_seed = seed;
	m_max  = 2147483647;
}

int Random::Next()
{
	// IBM's fastrand implementation.
	m_seed = (214013 * m_seed + 2531011); 
	return (m_seed >> 16) & 0x7FFF; 
}

int Random::Next(int loBound, int hiBound)
{
	return Next() % (hiBound + loBound - 1) + loBound;
}

double Random::Next_Double()
{
	return (float)Next() / (float)m_max;
}

double Random::Next_Double(int loBound, int hiBound)
{
	return loBound + (Next_Double() * (hiBound - loBound - 1));
}
