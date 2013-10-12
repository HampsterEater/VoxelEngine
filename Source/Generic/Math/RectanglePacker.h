// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MATH_RECTANGLEPACKER_
#define _GENERIC_MATH_RECTANGLEPACKER_

#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Point.h"

#include <vector>

class RectanglePacker
{
private:
	int m_width;
	int m_height;
	int m_spacing;

	int m_max_row_height;
	int m_offset_x;
	int m_offset_y;

public:
	RectanglePacker();
	RectanglePacker(int width, int height);

	bool Pack(Point size, Rect& location);

};

#endif