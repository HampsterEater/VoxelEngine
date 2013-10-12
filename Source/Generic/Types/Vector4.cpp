// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Types\Vector4.h"

#include <math.h>

Vector4::Vector4()
	: X(0)
	, Y(0)
	, Z(0)
	, W(0)
{
}

Vector4::Vector4(float x, float y, float z, float w)
	: X(x)
	, Y(y)
	, Z(z)
	, W(w)
{
}

Vector4::Vector4(Vector3 v, float w)
	: X(v.X)
	, Y(v.Y)
	, Z(v.Z)
	, W(w)
{
}