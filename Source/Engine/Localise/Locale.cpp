// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Localise\Locale.h"
#include "Engine\IO\StreamFactory.h"

#include "Generic\Helper\StringHelper.h"

Language::Language()
	: Source_Buffer(NULL)
	, XML_Document(new rapidxml::xml_document<>())
	, Short_Name("")
	, Long_Name("")
	, URL(NULL)
{
}

Language::~Language()
{
	SAFE_DELETE(XML_Document);
	SAFE_DELETE(Source_Buffer);
}

void Language::Reload()
{
	Load(URL);
}

bool Language::Load(const char* url)
{
	URL = url;

	// Open file.
	Stream* stream = StreamFactory::Open(url, StreamMode::Read);
	if (stream == NULL)
	{
		return false;
	}

	// Load source in a single string.
	int source_len = stream->Length();

	// TODO: Resize/Reuse old buffer, rather than allocating new one.
	if (Source_Buffer != NULL)
	{
		SAFE_DELETE(Source_Buffer);
	}

	Source_Buffer = new char[source_len + 1];
	Source_Buffer[source_len] = '\0';
	if (Source_Buffer == NULL)
	{
		DBG_LOG("Could not xml-source string into memory.");
		SAFE_DELETE(stream);
		return false;
	}
	stream->Read(Source_Buffer, 0, source_len);

	// Try and parse XML.
	try
	{
		XML_Document->parse<0>(Source_Buffer);
	}
	catch (rapidxml::parse_error error)
	{
		const char* offset = error.where<char>();
		int line = 0;
		int column = 0;

		StringHelper::Find_Line_And_Column(Source_Buffer, offset - Source_Buffer, line, column);

		DBG_LOG("Failed to parse XML with error @ %i:%i: %s", line, column, error.what());
		SAFE_DELETE(stream);
		return false;
	}

	// Unpack language data.
	if (!Unpack())
	{
		SAFE_DELETE(stream);
		return false;
	}

	return true;
}

bool Language::Unpack()
{
	const rapidxml::xml_node<>* element = XML_Document->first_node("xml");
	if (element == NULL)
	{
		return false;
	}

	const rapidxml::xml_node<>* short_name_element = element->first_node("string", 0, false);
	while (short_name_element != NULL)
	{
		rapidxml::xml_attribute<>* id_element = short_name_element->first_attribute("id", 0, false);
		if (id_element == NULL)
		{
			DBG_LOG("Language string missing id.");
			continue;
		}

		unsigned int id = StringHelper::Hash(id_element->value());
		const char* value = short_name_element->value();

		Strings.Set(id, value);

		short_name_element = short_name_element->next_sibling("string", 0, false);
	}

	// Get mandatory strings.
	Long_Name  = Strings.Get(StringHelper::Hash("name_long"));
	if (Long_Name == NULL)
	{
		DBG_LOG("Language file missing required string 'name_long'");
		return false;
	}

	Short_Name = Strings.Get(StringHelper::Hash("name_short"));
	if (Long_Name == NULL)
	{
		DBG_LOG("Language file missing required string 'name_short'");
		return false;
	}

	DBG_LOG("Loaded langauge: %s (%s)", Long_Name, Short_Name);

	return true;
}

Locale::Locale()
	: m_current_language(NULL)
{
}

Locale::~Locale()
{
	m_languages.Clear();
}

bool Locale::Load_Language(const char* name)
{
	// New language instance.
	Language* language = new Language();
	if (!language->Load(name))
	{
		SAFE_DELETE(language);
		return false;
	}
	
	// Insert language.
	m_languages.Set(StringHelper::Hash(language->Short_Name), language);

	return true;
}

bool Locale::Change_Language(const char* name)
{
	Language* language = m_languages.Get(StringHelper::Hash(name));
	if (language == NULL)
	{
		return false;
	}
	
	DBG_LOG("Changed to langauge: %s (%s)", language->Long_Name, language->Short_Name);

	m_current_language = language;
	return true;
}

const char* Locale::Get_String(std::string& id)
{
	return Get_String(id.c_str());
}

const char* Locale::Get_String(const char* id)
{
	if (id[0] == '#')
	{
		return Get_String(StringHelper::Hash(id + 1));
	}
	else
	{
		return id;
	}
}

const char* Locale::Get_String(int id)
{
	DBG_ASSERT(m_current_language != NULL);
	const char* str = m_current_language->Strings.Get(id);
	if (str == NULL)
	{
		DBG_LOG("Missing string for hash code : 0x%08x", id);
		str = "##_MISSING_STRING_##";
		m_current_language->Strings.Set(id, "##_MISSING_STRING_##");
	}
	return str;
}