// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_POINT_
#define _GENERIC_POINT_

class Point
{
public:
	float X, Y;

	Point()
		: X(0)
		, Y(0)
	{
	}
	Point(float x, float y)
	{
		X = x;
		Y = y;
	}
};

#endif