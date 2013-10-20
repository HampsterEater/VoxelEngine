// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_RECTANGLE_
#define _GENERIC_RECTANGLE_

#include <string>

class Rect
{
public:
	float X, Y, Width, Height;

	Rect();
	Rect(float x, float y, float w, float h);

	// To/From string.
	std::string To_String();
	static bool Parse(std::string value, Rect& result);

};

#endif