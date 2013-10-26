// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_CRYPT_CRC32_
#define _GENERIC_CRYPT_CRC32_

class CRC32
{
private:
	CRC32(); // Static class.

public:
	static u32 Calculate(void* buffer, int size);

};

#endif
