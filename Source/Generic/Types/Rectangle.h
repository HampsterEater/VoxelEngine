// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_RECTANGLE_
#define _GENERIC_RECTANGLE_

class Rect
{
public:
	float X, Y, Width, Height;

	Rect()
		: X(0)
		, Y(0)
		, Width(0)
		, Height(0)
	{
	}

	Rect(float x, float y, float w, float h)
	{
		X = x;
		Y = y;
		Width = w;
		Height = h;
	}
};

#endif