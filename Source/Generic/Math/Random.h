// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MATH_RANDOM_
#define _GENERIC_MATH_RANDOM_

class Random
{
private:
	int m_seed;
	int m_max;

public:
	Random(int seed);

	int Next();
	int Next(int loBound, int hiBound);

	double Next_Double();
	double Next_Double(int loBound, int hiBound);

};

#endif