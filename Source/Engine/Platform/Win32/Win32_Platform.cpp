// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Platform\Win32\Win32_Platform.h"
#include "Generic\Helper\StringHelper.h"

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
		return (float)GetTickCount();
	}
	else
	{
		LARGE_INTEGER tickCount;
		QueryPerformanceCounter(&tickCount);

		float f = (float)freq.QuadPart / 1000.0;
		return float(tickCount.QuadPart - start.QuadPart) / f;
	}	
}

void Win32_Platform::Crack_Path(const char* path, std::vector<std::string>& segments)
{
	StringHelper::Split(path, '/', segments);
}

bool Win32_Platform::Is_Directory(const char* path)
{
	DWORD flags = GetFileAttributesA(path);

	if (flags == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	if ((flags & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		return false;
	}

	return true;
}

bool Win32_Platform::Create_Directory(const char* path, bool recursive)
{
	if (recursive == true)
	{
		std::vector<std::string> cracked;
		std::string crack_path = "";

		Crack_Path(path, cracked);
		
		for (int i = 0; i < (int)cracked.size(); i++)
		{
			if (crack_path != "")
			{
				crack_path += "/";
			}
			crack_path += cracked.at(i);
			
			if (!Is_Directory(crack_path.c_str()))
			{
				bool result = Create_Directory(crack_path.c_str(), false);
				if (result == false)
				{
					return false;
				}
			}
		}

		return true;
	}
	else
	{
		int result = CreateDirectoryA(path, NULL);
		return (result != 0);
	}
}
