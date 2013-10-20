// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_INTVECTOR3_
#define _GENERIC_INTVECTOR3_

#include <string>

struct IntVector3
{
public:
	int X, Y, Z;

	// Constructors.
	IntVector3();
	IntVector3(int x, int y, int z);

	// Overloaded operators.
	bool operator==(const IntVector3 &other) const;
	bool operator!=(const IntVector3 &other) const;
	IntVector3 operator- (const IntVector3 a) const;
	
	// General.
	float Length_Squared() const;

	// To/From string.
	std::string To_String();
	static bool Parse(std::string value, IntVector3& result);

};

#endif