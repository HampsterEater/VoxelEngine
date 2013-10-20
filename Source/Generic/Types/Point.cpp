// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Types\Point.h"
#include "Generic\Helper\StringHelper.h"

Point::Point()
	: X(0)
	, Y(0)
{
}

Point::Point(float x, float y)
{
	X = x;
	Y = y;
}

std::string Point::To_String()
{
	return StringHelper::To_String(X) + "," + StringHelper::To_String(Y);
}

bool Point::Parse(std::string value, Point& result)
{
	std::vector<std::string> segments;
	StringHelper::Split(value.c_str(), ',', segments);

	if (segments.size() != 2)
		return false;

	result = Point(
				(float)atof(segments.at(0).c_str()),
				(float)atof(segments.at(1).c_str())
			 );

	return true;
}
