// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Types\IntVector3.h"

#include <math.h>

IntVector3::IntVector3()
	: X(0)
	, Y(0)
	, Z(0)
{
}

IntVector3::IntVector3(int x, int y, int z)
	: X(x)
	, Y(y)
	, Z(z)
{
}

bool IntVector3::operator==(const IntVector3 &other) const
{
	return X == other.X && Y == other.Y && Z == other.Z;
}

bool IntVector3::operator!=(const IntVector3 &other) const
{
	return !operator==(other);
}

IntVector3 IntVector3::operator-(const IntVector3 a) const
{
	return IntVector3(X - a.X, Y - a.Y, Z - a.Z);
}
	
float IntVector3::Length_Squared() const
{
	return (X * X + Y * Y + Z * Z);
}