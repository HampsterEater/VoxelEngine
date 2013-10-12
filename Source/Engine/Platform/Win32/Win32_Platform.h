// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_PLATFORM_WIN32_PLATFORM_
#define _ENGINE_PLATFORM_WIN32_PLATFORM_

#include "Generic\Patterns\Singleton.h"
#include "Engine\Platform\Platform.h"

class Win32_Platform : public Platform
{
public:

	// Time functions.
	float Get_Ticks();

	// Path functions.
	void Crack_Path(const char* path, std::vector<std::string>& segments);
	std::string Join_Path(std::string a, std::string b);
	std::string Extract_Directory(std::string a);
	std::string Extract_Filename(std::string a);

	// Directory functions.
	bool Is_File(const char* path);
	bool Is_Directory(const char* path);
	bool Create_Directory(const char* path, bool recursive);

private:
	friend class Platform;

	Win32_Platform();

};

#endif

