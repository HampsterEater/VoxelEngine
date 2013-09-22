// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Voxels\Serialization\WorldFile.h"
#include "Engine\IO\StreamFactory.h"

// World File Format
//
//		<header>
//

WorldFile::WorldFile(ChunkManager* manager, const ChunkManagerConfig& config)
	: m_manager(manager)
	, m_config(config)
{
}

WorldFile::~WorldFile()
{
}

// General properties.
bool WorldFile::Open()
{
	DBG_LOG("Opening world file: %s", m_config.world_file_path);

	m_stream = StreamFactory::Open(m_config.world_file_path, (StreamMode::Type)(StreamMode::Read | StreamMode::Write));

	// Wut? We must not have permission to write to the file system!
	if (m_stream == NULL)
	{
		return false;
	}

	// If stream is empty its new, so fill it with current settings and jump out.
	if (m_stream->Length() == 0)
	{
		Setup_Default_Header();
		Flush();
		return true;
	}

	// Check we have enough stream to load the header!
	if (m_stream->Length() < sizeof(Header))
	{	
		DBG_LOG("World header is corrupt.");

		Close();
		return false;
	}

	// Read in header.
	m_stream->Read((char*)&m_header, 0, sizeof(Header));

	// Validate header.
	if (!Validate_Header())
	{
		DBG_LOG("World header validation failed.");

		Close();
		return false;
	}

	return true;
}

void WorldFile::Setup_Default_Header()
{
	m_header.magic		= Magic;
	m_header.version	= Version;
	m_header.world_hash = rand();
}

bool WorldFile::Validate_Header()
{
	if (m_header.magic != Magic)
		return false;

	if (m_header.version != Version)
		return false;

	return true;
}

void WorldFile::Flush()
{
	m_stream->Write((char*)&m_header, 0, sizeof(Header));
	m_stream->Flush();
}

void WorldFile::Close()
{
	SAFE_DELETE(m_stream);
}

WorldFile::Header WorldFile::Get_Header()
{
	return m_header;
}