// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic\Helper\StringHelper.h"

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
			start_offset--;
			break;
		}
	}
	
	for (end_offset = length - 1; end_offset >= 0; end_offset--)
	{
		if (!iswspace(value[end_offset]))
		{
			start_offset++;
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
