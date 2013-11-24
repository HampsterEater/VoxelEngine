// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_RECTANGLE_
#define _GENERIC_RECTANGLE_

#include <string>

#include "Generic\Types\Point.h"

class Rect
{
public:
	float X, Y, Width, Height;

	Rect();
	Rect(float x, float y, float w, float h);

	// To/From string.
	std::string To_String();
	static bool Parse(std::string value, Rect& result);

	bool Intersects(Point p);

};

#endif