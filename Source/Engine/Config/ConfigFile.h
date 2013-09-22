// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CONFIGFILE_
#define _ENGINE_CONFIGFILE_

#include "Generic\Types\IntVector3.h"
#include "Generic\Types\Vector3.h"

#include "Generic\Helper\StringHelper.h"

#include "Generic\ThirdParty\RapidXML\rapidxml.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_iterators.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_utils.hpp"
 
class ConfigFile
{
private:
	rapidxml::xml_document<>*	m_xml_document;
	char*						m_source_buffer;
	int							m_source_buffer_len;

private:

	// Memory management.
	bool Resize_Buffer(int size);

public:
	
	// Constructor!
	ConfigFile();
	~ConfigFile();

	// Save & load options.
	//bool Save(const char* path);
	bool Load(const char* path);
	
	// Used by derived classes to unpack and repack data members for saving/loading.
	//virtual void Pack  (ConfigFile& file);
	virtual void Unpack(const ConfigFile& file);

	// ==============================================================
	// Get functions
	// ==============================================================
	template<typename T>
	T Get(const char* name) const;
	
	template<>
	const char* Get<const char*>(const char* name) const
	{
		const rapidxml::xml_node<>* element = m_xml_document->first_node("xml");
		DBG_ASSERT(element != NULL);

		const int name_len = strlen(name);
		std::string node_name = "";

		for (int i = 0; i < name_len; i++)
		{
			char chr = name[i];
			if (chr == '\\' || chr == '/')
			{
				element = element->first_node(node_name.c_str(), 0, false);
				DBG_ASSERT(element != NULL);

				node_name = "";
			}
			else
			{
				node_name += chr;
			}
		}

		if (node_name != "")
		{
			element = element->first_node(node_name.c_str(), 0, false);
			DBG_ASSERT(element != NULL);
		}

		return element->value();
	}

	template<>
	int Get<int>(const char* name) const
	{
		const char* source = Get<const char*>(name);
		return atoi(source);
	}
	
	template<>
	bool Get<bool>(const char* name) const
	{
		const char* source = Get<const char*>(name);
		return (stricmp(source, "0") == 0 || stricmp(source, "false") == 0) ? false : true;
	}

	template<>
	float Get<float>(const char* name) const
	{
		const char* source = Get<const char*>(name);
		return (float)atof(source);
	}
	
	template<>
	IntVector3 Get<IntVector3>(const char* name) const
	{
		const char* source = Get<const char*>(name);

		std::vector<std::string> segments;
		StringHelper::Split(source, ',', segments);

		DBG_ASSERT_STR(segments.size() == 3, "XML attribute %s expected in format vector format; int,int,int");

		return IntVector3(
					atoi(segments.at(0).c_str()),
					atoi(segments.at(1).c_str()),
					atoi(segments.at(2).c_str())
			   );
	}
	
	template<>
	Vector3 Get<Vector3>(const char* name) const
	{
		const char* source = Get<const char*>(name);

		std::vector<std::string> segments;
		StringHelper::Split(source, ',', segments);

		DBG_ASSERT_STR(segments.size() == 3, "XML attribute %s expected in format vector format; float,float,float");

		return Vector3(
					(float)atof(segments.at(0).c_str()),
					(float)atof(segments.at(1).c_str()),
					(float)atof(segments.at(2).c_str())
			   );
	}

	// ==============================================================
	// Set functions
	// ==============================================================
	/*
	template<typename T>
	void Set(const char* name, T value);

	template<>
	void Set<const char*>(const char* name, const char* value)
	{
		rapidxml::xml_node<>* element = m_xml_document.first_node("xml");
		DBG_ASSERT(element != NULL);

		const int name_len = strlen(name);
		std::string node_name = "";

		for (int i = 0; i < name_len; i++)
		{
			char chr = name[i];
			if (chr == '\\' || chr == '/')
			{
				element = element->first_node(node_name.c_str(), 0, false);
				DBG_ASSERT(element != NULL);

				node_name = "";
			}
			else
			{
				node_name += chr;
			}
		}

		if (node_name != "")
		{
			element = element->first_node(node_name.c_str(), 0, false);
			DBG_ASSERT(element != NULL);
		}

		element->value(value);
	}

	template<>
	void Set<int>(const char* name, int value)
	{
		(value);
	}
	
	template<>
	void Set<bool>(const char* name, bool value)
	{
		(value);
	}

	template<>
	void Set<float>(const char* name, float value)
	{
		(value);
	}

	template<>
	void Set<IntVector3>(const char* name, IntVector3 value)
	{
		(value);
	}

	template<>
	void Set<Vector3>(const char* name, Vector3 value)
	{
		(value);
	}
	*/

};

#endif

