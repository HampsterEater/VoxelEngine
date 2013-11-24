// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_POINT_
#define _GENERIC_POINT_

#include <string>

class Point
{
public:
	float X, Y;

	Point();
	Point(float x, float y);
	
	// Operators.
	bool operator==(Point& rhs) const;
	bool operator!=(Point& rhs) const;

	Point Round() const;

	// To/From string.
	std::string To_String();
	static bool Parse(std::string value, Point& result);

};

#endif