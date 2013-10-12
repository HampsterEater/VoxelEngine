// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_VECTOR4_
#define _GENERIC_VECTOR4_

#include "Generic\Types\Vector3.h"

struct Vector4
{
public:
	float X, Y, Z, W;

	// Constructors.
	Vector4();
	Vector4(float x, float y, float z, float w);
	Vector4(Vector3 v, float w);

};

#endif