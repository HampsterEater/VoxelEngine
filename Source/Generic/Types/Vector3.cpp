// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Types\Vector3.h"

#include <math.h>

Vector3::Vector3()
	: X(0)
	, Y(0)
	, Z(0)
{
}

Vector3::Vector3(float x, float y, float z)
	: X(x)
	, Y(y)
	, Z(z)
{
}

Vector3 Vector3::operator* (const float a) const
{
	return Vector3(X * a, Y * a, Z * a);
}

Vector3 Vector3::operator+ (const Vector3 a) const
{
	return Vector3(X + a.X, Y + a.Y, Z + a.Z);
}

Vector3 Vector3::operator/ (const Vector3 a) const
{
	return Vector3(X / a.X, Y / a.Y, Z / a.Z);
}

Vector3 Vector3::operator/ (const float a) const
{
	return Vector3(X / a, Y / a, Z / a);
}

Vector3 Vector3::operator- (const Vector3 a) const
{
	return Vector3(X - a.X, Y - a.Y, Z - a.Z);
}

Vector3 Vector3::operator- (const IntVector3 a) const
{
	return Vector3(X - a.X, Y - a.Y, Z - a.Z);
}

Vector3 Vector3::operator- () const
{
	return Vector3(-X, -Y, -Z);
}

Vector3 Vector3::Cross(Vector3 a) const
{
	return Vector3
		(
			Y * a.Z - Z * a.Y,
			Z * a.X - X * a.Z,
			X * a.Y - Y * a.X
		);
}

float Vector3::Product(Vector3 v) const
{
	return (X * v.X + Y * v.Y + Z * v.Z);
}

Vector3 Vector3::Normalize() const
{
	float len = 1.0f / sqrt((X * X) + (Y * Y) + (Z * Z));
	return Vector3(X * len, Y * len, Z * len);
}

float Vector3::Length() const
{
	return sqrt(X * X + Y * Y + Z * Z);
}

float Vector3::Length_Squared() const
{
	return X * X + Y * Y + Z * Z;
}

Vector3 Vector3::Direction() const
{
	return Vector3
		(
			cos(Z) * sin(Y),
			sin(Z),
			cos(Z) * cos(Y)
		);
}

Vector3 Vector3::Right() const
{
	return Vector3
		(
			sin(Y - 3.14f / 2.0f),
			0,
			cos(Y - 3.14f / 2.0f)
		);
}
