// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STREAM_WIN32_FILESTREAM_
#define _ENGINE_STREAM_WIN32_FILESTREAM_

#include "Engine\IO\Stream.h"

class Win32_FileStream : public Stream
{
private:
	FILE* m_handle;
	bool  m_open;

public:

	Win32_FileStream(FILE* handle);
	~Win32_FileStream();

	// Members that have to be overidden.
	bool			IsEOF		();
	unsigned int	Position	();
	void			Seek		(unsigned int offset);
	unsigned int	Length		();
	void			Write		(const char* buffer, int offset, int length);
	void			Read		(char* buffer, int offset, int length);
	void			Close		();

};

#endif

