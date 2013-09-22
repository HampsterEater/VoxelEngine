// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_PLATFORM_
#define _ENGINE_PLATFORM_

#include "Generic\Patterns\Singleton.h"

#include <string>
#include <vector>

class Platform : public Singleton<Platform>
{
public:
	static Platform* Create();

	// Time functions.
	virtual float Get_Ticks() = 0;
	
	// Path functions.
	virtual void Crack_Path(const char* path, std::vector<std::string>& segments) = 0;

	// Directory functions.
	virtual bool Is_Directory(const char* path) = 0;
	virtual bool Create_Directory(const char* path, bool recursive) = 0;
};

#endif

