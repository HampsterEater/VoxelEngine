// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Types\Rectangle.h"
#include "Generic\Helper\StringHelper.h"

Rect::Rect()
	: X(0)
	, Y(0)
	, Width(0)
	, Height(0)
{
}

Rect::Rect(float x, float y, float w, float h)
{
	X = x;
	Y = y;
	Width = w;
	Height = h;
}

bool Rect::Intersects(Point p)
{
	return (p.X >= X && p.Y >= Y && p.X < X + Width && p.Y < Y + Height);
}

std::string Rect::To_String()
{
	return StringHelper::To_String(X) + "," + StringHelper::To_String(Y) + "," + StringHelper::To_String(Width)+ "," + StringHelper::To_String(Height);	
}

bool Rect::Parse(std::string value, Rect& result)
{
	std::vector<std::string> segments;
	StringHelper::Split(value.c_str(), ',', segments);
	
	if (segments.size() != 4)
		return false;

	result = Rect(
				(float)atof(segments.at(0).c_str()),
				(float)atof(segments.at(1).c_str()),
				(float)atof(segments.at(2).c_str()),
				(float)atof(segments.at(3).c_str())
			);

	return true;
}
