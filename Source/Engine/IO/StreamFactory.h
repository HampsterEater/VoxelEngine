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

	// Constructors
	StreamFactory();	
	~StreamFactory();	

	// Derived factory methods.
	virtual Stream*	Try_Open(const char* url, StreamMode::Type mode) = 0;

};

#endif

