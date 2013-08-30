// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Math\Math.h"

float Math::Lerp(float v0, float v1, float d)
{
	return v0 + (v1 - v0) * d;
}

float Math::Lerp(float v00, float v10, float v01, float v11, float dx, float dy)
{
	float u = Lerp(v00, v10, dx);
	float v = Lerp(v01, v11, dx);
	return Lerp(u, v, dy);
}

float Math::Lerp(float v000, float v100, float v010, float v110, float v001, float v101, float v011, float v111, float dx, float dy, float dz)
{	
	float u = Lerp(v000, v100, v010, v110, dx, dy);
	float v = Lerp(v001, v101, v011, v111, dx, dy);
	return Lerp(u, v, dz);
}
