// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_COLOR_
#define _GENERIC_COLOR_

#include "Generic\Types\Vector3.h"
#include "Generic\Types\Vector4.h"

class Color
{
public:
	// Some commonly used colors.
	static Color White;
	static Color Black;
	static Color Red;
	static Color Green;
	static Color Blue;
	static Color Yellow;
	static Color Orange;
	static Color Magenta;

public:
	int R, G, B, A;

	Color();
	Color(int r, int g, int b, int a);

	Vector3 To_Vector3();
	Vector4 To_Vector4();
	
	// Operators.
	bool operator==(Color& rhs) const;
	bool operator!=(Color& rhs) const;

	static bool Parse(const char* value, Color& output);

};

#endif