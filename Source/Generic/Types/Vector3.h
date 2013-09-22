// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_VECTOR3_
#define _GENERIC_VECTOR3_

#include "Generic\Types\IntVector3.h"
#include "Generic\Types\Vector4.h"

struct Vector3
{
public:
	float X, Y, Z;

	// Constructors.
	Vector3();
	Vector3(float x, float y, float z);

	// Operator overloads.
	Vector3 operator* (const float a) const;
	Vector3 operator+ (const Vector3 a) const;
	Vector3 operator/ (const Vector3 a) const;
	Vector3 operator- (const Vector3 a) const;
	Vector3 operator- (const IntVector3 a) const;
	
	// General.
	float   Product(Vector3 v) const;
	Vector3	Cross(Vector3 a) const;
	Vector3 Normalize() const;
	float	Length() const;
	float	Length_Squared() const;
	Vector4 To_Vector4() const;

};

#endif