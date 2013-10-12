// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STREAMFACTORY_
#define _ENGINE_STREAMFACTORY_

#include "Generic\Types\LinkedList.h"
#include "Engine\IO\Stream.h"

class StreamFactory
{
private:
	static LinkedList<StreamFactory*> m_factories;

public:
		
	// Static methods.
	static Stream* Open(const char* url, StreamMode::Type mode);
	static u64     Get_Last_Modified(const char* url);

	// Constructors
	StreamFactory();	
	virtual ~StreamFactory();	

	// Derived factory methods.
	virtual Stream*	Try_Open(const char* url, StreamMode::Type mode) = 0;
	virtual u64     Try_Get_Last_Modified(const char* url) = 0;

};

#endif

