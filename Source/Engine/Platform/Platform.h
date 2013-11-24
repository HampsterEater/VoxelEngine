// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_PLATFORM_
#define _ENGINE_PLATFORM_

#include "Generic\Patterns\Singleton.h"

#include <string>
#include <vector>

class GameRunner;

// This function is overloaded by game instances to return
// the game runner instance for their game.
extern GameRunner* New_Game();
extern void Print_Game_Version();

// Debugging structures.
struct StackFrame
{
	enum
	{
		MAX_STRING_SIZE = 512
	};

	int				Address;
	char			File[MAX_STRING_SIZE];
	char			Name[MAX_STRING_SIZE];
	int				Line;
	int				Column;
};

class Platform : public Singleton<Platform>
{
public:
	static Platform* Create();

	// Time functions.
	virtual float Get_Ticks() = 0;
	
	// Path functions.
	virtual void		Crack_Path			(const char* path, std::vector<std::string>& segments) = 0;
	virtual std::string Join_Path			(std::string a, std::string b) = 0;
	virtual std::string Extract_Directory	(std::string a) = 0;
	virtual std::string Extract_Filename	(std::string a) = 0;
	virtual std::string	Get_Working_Dir		() = 0;
	virtual void		Set_Working_Dir		(const char* path) = 0;

	// Directory functions.
	virtual bool Is_File					(const char* path) = 0;
	virtual bool Is_Directory				(const char* path) = 0;
	virtual bool Create_Directory			(const char* path, bool recursive) = 0;

	// Debug functions.
	virtual int  Get_Stack_Trace			(StackFrame* frames, int max_frames, void* platform_specific = NULL, int offset = 0) = 0;
	virtual void Resolve_Stack_Frame_Info	(StackFrame& frame) = 0;

};

#endif

