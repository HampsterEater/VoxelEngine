// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STREAM_WIN32_FILESTREAMFACTORY_
#define _ENGINE_STREAM_WIN32_FILESTREAMFACTORY_

#include "Engine\IO\StreamFactory.h"

class Win32_FileStreamFactory : public StreamFactory
{
public:
	Stream*	Try_Open(const char* url, StreamMode::Type mode);
	s64   Try_Get_Last_Modified(const char* url);

};

#endif

