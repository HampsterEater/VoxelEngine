// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CONFIGFILE_
#define _ENGINE_CONFIGFILE_

#include "Generic\Types\IntVector3.h"
#include "Generic\Types\Vector3.h"
#include "Generic\Types\Point.h"
#include "Generic\Types\Rectangle.h"

#include "Generic\Helper\StringHelper.h"

#include "Generic\ThirdParty\RapidXML\rapidxml.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_iterators.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_utils.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_print.hpp"
 
typedef rapidxml::xml_node<>* ConfigFileNode;

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
	bool Save(const char* path);
	bool Load(const char* path);
	
	// Used by derived classes to unpack and repack data members for saving/loading.
	virtual void Pack  (ConfigFile& file);
	virtual void Unpack(ConfigFile& file);
	
	// ==============================================================
	// Node functions
	// ==============================================================
	ConfigFileNode Get_Node(const char* name, bool create = false, bool force_create_last = false)
	{
		rapidxml::xml_node<>* element = m_xml_document->first_node("xml");
		if (element == NULL)
		{
			if (create == true)
			{
				element = m_xml_document->allocate_node(rapidxml::node_element, m_xml_document->allocate_string("xml"), NULL);
				m_xml_document->append_node(element);
			}
			else
			{
				DBG_ASSERT_STR(false, "Expecting config node '%s'.", "xml");
			}
		}

		const int name_len = strlen(name);
		std::string node_name = "";

		for (int i = 0; i < name_len; i++)
		{
			char chr = name[i];
			if (chr == '\\' || chr == '/')
			{
				rapidxml::xml_node<>* new_element = element->first_node(node_name.c_str(), 0, false);
				if (new_element == NULL)
				{
					if (create == true)
					{
						new_element = m_xml_document->allocate_node(rapidxml::node_element, m_xml_document->allocate_string(node_name.c_str()), NULL);
						element->append_node(new_element);
					}
					else
					{
						DBG_ASSERT_STR(false, "Expecting config node '%s'.", node_name.c_str());
					}
				}

				element = new_element;
				node_name = "";
			}
			else
			{
				node_name += chr;
			}
		}

		if (node_name != "")
		{
			rapidxml::xml_node<>* new_element = NULL;

			if (force_create_last == true)
			{
				new_element = m_xml_document->allocate_node(rapidxml::node_element, m_xml_document->allocate_string(node_name.c_str()), NULL);
				element->append_node(new_element);
			}
			else
			{
				new_element = element->first_node(node_name.c_str(), 0, false);
				if (new_element == NULL)
				{
					if (create == true)
					{
						new_element = m_xml_document->allocate_node(rapidxml::node_element, m_xml_document->allocate_string(node_name.c_str()), NULL);
						element->append_node(new_element);
					}
					else
					{
						DBG_ASSERT_STR(false, "Expecting config node '%s'.", node_name.c_str());
					}
				}
			}

			element = new_element;
		}

		return element;
	}
	
	ConfigFileNode New_Node(const char* name)
	{
		return Get_Node(name, true, true);
	}

	// ==============================================================
	// Get functions
	// ==============================================================
	template<typename T>
	T Get(const char* name, ConfigFileNode node = NULL, bool as_attribute = false)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		T result;
		if (!T::Parse(source, result))
		{
			DBG_ASSERT_STR(false, "Config node '%s' was in invalid format.", name);
		}
		return result;
	}
	
	template<>
	const char* Get<const char*>(const char* name, ConfigFileNode node, bool as_attribute)
	{
		if (as_attribute != NULL)
		{
			DBG_ASSERT(node != NULL);
			
			rapidxml::xml_attribute<>* attribute = node->first_attribute(name, 0, false);			
			DBG_ASSERT_STR(attribute != NULL, "Expecting config node attribute '%s'.", name);

			return attribute->value();
		}
		else
		{
			DBG_ASSERT(node == NULL);

			ConfigFileNode n = Get_Node(name);
			return n->value();
		}
	}

	template<>
	int Get<int>(const char* name, ConfigFileNode node, bool as_attribute)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		return atoi(source);
	}
	
	template<>
	unsigned int Get<unsigned int>(const char* name, ConfigFileNode node, bool as_attribute)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		return (unsigned int)atoi(source);
	}

	template<>
	bool Get<bool>(const char* name, ConfigFileNode node, bool as_attribute)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		return (stricmp(source, "0") == 0 || stricmp(source, "false") == 0) ? false : true;
	}

	template<>
	float Get<float>(const char* name, ConfigFileNode node, bool as_attribute)
	{
		const char* source = Get<const char*>(name, node, as_attribute);
		return (float)atof(source);
	}

	template<>
	std::vector<const char*> Get<std::vector<const char*>>(const char* name, ConfigFileNode node, bool as_attribute)
	{
		std::vector<const char*> result;

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
				DBG_ASSERT_STR(element != NULL, "Expecting config node '%s'.", node_name.c_str());

				node_name = "";
			}
			else
			{
				node_name += chr;
			}
		}

		// Iterate over results.
		DBG_ASSERT(node_name != "");

		element = element->first_node(node_name.c_str(), 0, false);
		while (element != NULL)
		{
			result.push_back(element->value());
			element = element->next_sibling(node_name.c_str(), 0, false);
		}
	
		return result;		
	}
	
	template<>
	std::vector<ConfigFileNode> Get<std::vector<ConfigFileNode>>(const char* name, ConfigFileNode node, bool as_attribute)
	{
		std::vector<ConfigFileNode> result;

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
				DBG_ASSERT_STR(element != NULL, "Expecting config node '%s'.", node_name.c_str());

				node_name = "";
			}
			else
			{
				node_name += chr;
			}
		}

		// Iterate over results.
		DBG_ASSERT(node_name != "");

		element = element->first_node(node_name.c_str(), 0, false);
		while (element != NULL)
		{
			result.push_back((ConfigFileNode)element);
			element = element->next_sibling(node_name.c_str(), 0, false);
		}
	
		return result;		
	}

	// ==============================================================
	// Set functions
	// ==============================================================
	template<typename T>
	void Set(const char* name, T value, ConfigFileNode node = NULL, bool as_attribute = false)
	{
		Set(name, value.To_String().c_str(), node, as_attribute);		
	}

	template<>
	void Set<const char*>(const char* name, const char* value, ConfigFileNode node, bool as_attribute)
	{
		if (node == NULL)
		{
			node = Get_Node(name, true);
		}
		/*else
		{
			DBG_ASSERT(as_attribute == true);
		}*/

		if (as_attribute == true)
		{
			rapidxml::xml_attribute<>* attr = node->first_attribute(name, 0, false);
			if (attr == NULL)
			{
				attr = m_xml_document->allocate_attribute(m_xml_document->allocate_string(name), m_xml_document->allocate_string(value));
				node->append_attribute(attr);
			}
			attr->value(m_xml_document->allocate_string(value));
		}
		else
		{
			node->value(m_xml_document->allocate_string(value));
		}
	}

	template<>
	void Set<int>(const char* name, int value, ConfigFileNode node, bool as_attribute)
	{
		Set(name, StringHelper::To_String(value).c_str(), node, as_attribute);
	}

	template<>
	void Set<unsigned int>(const char* name, unsigned int value, ConfigFileNode node, bool as_attribute)
	{
		Set(name, StringHelper::To_String(value).c_str(), node, as_attribute);
	}
	
	template<>
	void Set<bool>(const char* name, bool value, ConfigFileNode node, bool as_attribute)
	{
		Set(name, value == true ? "true" : "false", node, as_attribute);
	}

	template<>
	void Set<float>(const char* name, float value, ConfigFileNode node, bool as_attribute)
	{
		Set(name, StringHelper::To_String(value).c_str(), node, as_attribute);
	}

};

#endif

