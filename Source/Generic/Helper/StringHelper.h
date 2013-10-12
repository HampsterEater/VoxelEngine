// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_HELPER_STRINGHELPER_
#define _GENERIC_HELPER_STRINGHELPER_

#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <map>

class StringHelper
{
private:
	StringHelper();

public:
	static int Split(const char* value, char deliminator, std::vector<std::string>& segments);
	static void Find_Line_And_Column(const char* text, int offset, int& line, int& column);
	static std::string Trim(const char* value);
	static std::string Remove_Whitespace(const char* value);

	static int Hash(const char* value);

	template<typename T>
	static std::string To_String(T value)
	{		
		std::stringstream ss;
		ss << value;
		return ss.str();
	}

};

#endif