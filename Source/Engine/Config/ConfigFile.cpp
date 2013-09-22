// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Config\ConfigFile.h"
#include "Engine\IO\StreamFactory.h"

#include "Generic\Helper\StringHelper.h"

ConfigFile::ConfigFile()
	: m_source_buffer(NULL)
	, m_source_buffer_len(0)
	, m_xml_document(new rapidxml::xml_document<>())
{
}

ConfigFile::~ConfigFile()
{
	SAFE_DELETE(m_xml_document);
	SAFE_DELETE(m_source_buffer);
}

bool ConfigFile::Resize_Buffer(int size)
{
	if (m_source_buffer == NULL || m_source_buffer_len < size)
	{
		if (m_source_buffer != NULL)
		{
			SAFE_DELETE(m_source_buffer);
		}

		m_source_buffer = new char[size + 1];
		m_source_buffer[size] = '\0';

		if (m_source_buffer == NULL)
		{
			return false;
		}
	}

	return true;
}

/*
bool ConfigFile::Save(const char* path)
{
	Stream* stream = StreamFactory::Open(path, (StreamMode::Type)(StreamMode::Write | StreamMode::Truncate));
	if (stream == NULL)
	{
		return false;
	}

	// Pack data.
	Unpack(*this);

	// Save data.
	const char* value = m_xml_document.value();
	stream->WriteString(value);

	// Clean up and return.
	delete stream;
	return true;
}
*/

bool ConfigFile::Load(const char* path)
{
	Stream* stream = StreamFactory::Open(path, StreamMode::Read);
	if (stream == NULL)
	{
		return false;
	}

	// Load source in a single string.
	int source_len = stream->Length();
	if (!Resize_Buffer(source_len))
	{
		DBG_LOG("Could not xml-source string into memory.");
		delete stream;
		return false;
	}
	stream->Read(m_source_buffer, 0, source_len);

	// Try and parse XML.
	try
	{
		m_xml_document->parse<0>(m_source_buffer);
	}
	catch (rapidxml::parse_error error)
	{
		const char* offset = error.where<char>();
		int line = 0;
		int column = 0;

		StringHelper::Find_Line_And_Column(m_source_buffer, offset - m_source_buffer, line, column);

		DBG_LOG("Failed to parse XML with error @ %i:%i: %s", line, column, error.what());
		delete stream;
		return false;
	}

	// Unpack data.
	Unpack(*this);

	// Clean up and return.
	delete stream;
	return true;
}
	
void ConfigFile::Unpack(const ConfigFile& file)
{
}

/*
void ConfigFile::Pack(ConfigFile& file)
{
}
*/
