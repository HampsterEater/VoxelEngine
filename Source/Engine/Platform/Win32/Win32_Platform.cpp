// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Platform\Win32\Win32_Platform.h"

#include <windows.h>

Win32_Platform::Win32_Platform()
{
}

float Win32_Platform::Get_Ticks()
{
	static LARGE_INTEGER freq;			
	static LARGE_INTEGER start;
	static int state = 0; // 0=not initialized, -1=not supported, 1=supported

	if (state == 0)
	{
		if (QueryPerformanceFrequency(&freq) == 0)
		{
			state = -1;
		}
		else
		{
			QueryPerformanceCounter(&start);
			state = 1;
		}
	}

	// Not supported, use tick count.
	if (state == -1)
	{
		return GetTickCount();
	}
	else
	{
		LARGE_INTEGER tickCount;
		QueryPerformanceCounter(&tickCount);

		float f = (float)freq.QuadPart / 1000.0;
		return float(tickCount.QuadPart - start.QuadPart) / f;
	}	
}