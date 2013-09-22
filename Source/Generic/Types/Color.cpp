// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Types\Color.h"

Color Color::White(255, 255, 255, 255);
Color Color::Black(0, 0, 0, 255);
Color Color::Red(255, 0, 0, 255);
Color Color::Green(0, 255, 0, 255);
Color Color::Blue(0, 0, 255, 255);
Color Color::Yellow(255, 255, 0, 255);
Color Color::Orange(255, 153, 0, 255);
Color Color::Magenta(255, 0, 255, 255);

Color::Color()
	: R(0)
	, G(0)
	, B(0)
	, A(0)
{
}

Color::Color(int r, int g, int b, int a)
	: R(r)
	, G(g)
	, B(b)
	, A(a)
{
}

Vector3 Color::To_Vector3()
{
	return Vector3(R / 255.0f, G / 255.0f, B / 255.0f);
}

Vector4 Color::To_Vector4()
{
	return Vector4(R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f);
}

bool Color::Parse(const char* value, Color& output)
{
	int r = 255, g = 255, b = 255, a = 255;
	int count = sscanf(value, "%d,%d,%d,%d", &r, &g, &b, &a);
	
	if (count >= 3)
	{
		output = Color(r, g, b, a);
		return true;
	}

	return false;
}
