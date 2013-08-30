// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\IO\Win32\Win32_FileStreamFactory.h"
#include "Engine\IO\Win32\Win32_FileStream.h"

#include <string>

Stream*	Win32_FileStreamFactory::Try_Open(const char* url, StreamMode::Type mode)
{
	std::string mode_string = "";

	if ((mode & StreamMode::Read) != 0)
	{
		mode_string += "r";
	}
	if ((mode & StreamMode::Write) != 0)
	{
		mode_string += "w";		
	}
	if ((mode & StreamMode::Truncate) != 0)
	{
		// We don't do shit for this on windows.
	}
	if ((mode & StreamMode::Create) != 0)
	{
		// We don't do shit for this on windows.
	}
	mode_string += "b";

	FILE* handle = fopen(url, mode_string.c_str());
	if (handle == NULL)
	{
		return NULL;
	}

	return new Win32_FileStream(handle);
}
