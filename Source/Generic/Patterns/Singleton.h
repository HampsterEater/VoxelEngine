// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_SINGLETON_
#define _GENERIC_SINGLETON_

template<typename T>
class Singleton
{
private:
	static T* m_pointer;

public:
	Singleton()
	{
		DBG_ASSERT(m_pointer == NULL);
		m_pointer = (T*)this;
	}

	~Singleton()
	{
		m_pointer = NULL;
	}

	static T* Get() 
	{
		DBG_ASSERT(m_pointer != NULL);
		return m_pointer;
	}

};

template<typename T>
T* Singleton<T>::m_pointer = NULL; 

#endif
