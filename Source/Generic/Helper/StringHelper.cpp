// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Helper\StringHelper.h"

#include <stdarg.h> 

int StringHelper::Split(const char* v, char deliminator, std::vector<std::string>& segments)
{
	std::string value = v;

	if (strlen(v) == 0)
	{
		return 0;
	}

	int startIndex = 0;

	while (true)
	{
		int offset = value.find(deliminator, startIndex);
		if (offset < 0)
		{
			break;
		}
	
		segments.push_back(value.substr(startIndex, offset - startIndex));

		startIndex = offset + 1;
	}

	segments.push_back(value.substr(startIndex, value.size() - startIndex));

	return segments.size();
}

void StringHelper::Find_Line_And_Column(const char* text, int offset, int& line, int& column)
{
	// Reset values.
	line = 1;
	column = 0;

	// Calculate length.
	//int len = strlen(text); // Assumption made that text length is inside bounds, as this function is used with strings containing multiple \0's
	for (int i = 0; /*i < len &&*/ i <= offset; i++)
	{
		char chr = text[i];
		if (chr == '\n')
		{
			line++;
			column = 0;
		}
		else if (chr != '\r')
		{
			column++;
		}
	}
}

std::string StringHelper::Trim(const char* value)
{
	int start_offset = 0;
	int end_offset = 0;
	int length = strlen(value);

	for (start_offset = 0; start_offset < length; start_offset++)
	{
		if (!iswspace(value[start_offset]))
		{
			break;
		}
	}
	
	for (end_offset = length - 1; end_offset >= 0; end_offset--)
	{
		if (!iswspace(value[end_offset]))
		{
			end_offset++;
			break;
		}
	}

	std::string result = std::string(value).substr(start_offset, end_offset - start_offset);

	return result;
}

std::string StringHelper::Remove_Whitespace(const char* value)
{
	std::string result = "";
	int length = strlen(value);

	for (int i = 0; i < length; i++)
	{
		if (!iswspace(value[i]))
		{
			result += value[i];
		}
	}
	
	return result;
}

int StringHelper::Hash(const char* value)
{
	unsigned int hash = 0;

	for (; *value; ++value)
	{
		hash += *value;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

std::string StringHelper::Format(const char* format, ...)
{
	va_list va;
	va_start(va, format);

	char buffer[512];
	int num = vsnprintf(buffer, 512, format, va);
	if (num >= 511)
	{
		char* new_buffer = new char[num + 1];
		vsnprintf(buffer, num + 1, format, va);
		delete[] new_buffer;
		
		va_end(va);
		return new_buffer;
	}

	va_end(va);

	return buffer;
}
