// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\IO\StreamFactory.h"

LinkedList<StreamFactory*> StreamFactory::m_factories;

Stream* StreamFactory::Open(const char* url, StreamMode::Type mode)
{
	for (LinkedList<StreamFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		StreamFactory* factory = *iter;
		Stream* result = factory->Try_Open(url, mode);
		if (result != NULL)
		{
			return result;
		}
	}
	return NULL;
}

StreamFactory::StreamFactory()
{
	m_factories.Add(this);
}

StreamFactory::~StreamFactory()
{
	m_factories.Remove(m_factories.Find(this));
}