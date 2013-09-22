// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Voxels\Serialization\RegionFile.h"
#include "Engine\IO\StreamFactory.h"

#include "Game\Scene\Voxels\ChunkManager.h"

// Region File Format
//
//		<header>
//		<chunk-existance-array>
//		<chunk-data>
//

RegionFile::RegionFile(IntVector3 position, ChunkManager* manager, const ChunkManagerConfig& config)
	: m_manager(manager)
	, m_config(config)
	, m_position(position)
{
	m_chunk_count		= m_config.region_size.X * m_config.region_size.Y * m_config.region_size.Z;
	m_chunk_data_size	= m_config.chunk_size.X * m_config.chunk_size.Y * m_config.chunk_size.Z * sizeof(Voxel); 
	m_chunk_exists		= new bool[m_chunk_count];
	memset(m_chunk_exists, 0, sizeof(bool) * m_chunk_count);

	m_chunk_origin = IntVector3(
		m_position.X * m_config.region_size.X,
		m_position.Y * m_config.region_size.Y,
		m_position.Z * m_config.region_size.Z
	);
	
	// Calculate data offset.
	m_voxel_data_offset = sizeof(Header) + (sizeof(bool) * m_chunk_count);

	// Create access mutex.
	m_access_mutex = Mutex::Create();
	DBG_ASSERT(m_access_mutex != NULL);
}

RegionFile::~RegionFile()
{
	SAFE_DELETE(m_access_mutex);

	SAFE_DELETE_ARRAY(m_chunk_exists);
}

// General properties.
bool RegionFile::Open()
{
	// TODO: Memory buffer overflow, write a cross platform safe version!
	char path[512];
	sprintf(path, "%s%0.8X.dat", m_config.region_directory, Flatten_Index(m_position.X, m_position.Y, m_position.Z));
	
	DBG_LOG("Opening region file: %s", path);

	m_stream = StreamFactory::Open(path, (StreamMode::Type)(StreamMode::Read | StreamMode::Write));

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

		// Fill stream with enough space to store all chunks in future.
		char* chunk_buffer = new char[m_chunk_data_size];
		for (int i = 0; i < m_chunk_count; i++)
		{
			m_stream->Write(chunk_buffer, 0, m_chunk_data_size);
		}
		SAFE_DELETE_ARRAY(chunk_buffer);

		return true;
	}

	// Check we have enough stream to load the header!
	if (m_stream->Length() < sizeof(Header))
	{
		DBG_LOG("Region header is corrupt.");

		Close();
		return false;
	}

	// Read in header.
	m_stream->Read((char*)&m_header, 0, sizeof(Header));

	// Read in chunk-existance array.
	m_stream->Read((char*)m_chunk_exists, 0, sizeof(bool) * m_chunk_count);

	// Validate header.
	if (!Validate_Header())
	{
		DBG_LOG("Region header validation failed.");

		Close();
		return false;
	}

	return true;
}

void RegionFile::Setup_Default_Header()
{
	m_header.magic		= Magic;
	m_header.version	= Version;
	m_header.world_hash = m_manager->Get_World_File()->Get_Header().world_hash;
}

bool RegionFile::Validate_Header()
{
	if (m_header.magic != Magic)
		return false;

	if (m_header.version != Version)
		return false;

	return true;
}

void RegionFile::Flush()
{
	m_stream->Seek(0);
	m_stream->Write((char*)&m_header, 0, sizeof(Header));
	m_stream->Write((char*)m_chunk_exists, 0, sizeof(bool) * m_chunk_count);
	m_stream->Flush();
}

void RegionFile::Close()
{
	SAFE_DELETE(m_stream);
}

RegionFile::Header RegionFile::Get_Header()
{
	return m_header;
}

bool RegionFile::Contains_Chunk(Chunk* chunk)
{
	MutexLock lock(m_access_mutex);

	IntVector3 position = chunk->Get_Position() - m_chunk_origin;
	int index = Flatten_Index(position.X, position.Y, position.Z);
	DBG_ASSERT(index >= 0 && index < m_chunk_count);

	return m_chunk_exists[index];
}

void RegionFile::Save_Chunk(Chunk* chunk)
{
	MutexLock lock(m_access_mutex);

	//DBG_LOG("Saving: %i,%i,%i\n", chunk->Get_Position().X, chunk->Get_Position().Y, chunk->Get_Position().Z);

	IntVector3 position = chunk->Get_Position() - m_chunk_origin;
	int index = Flatten_Index(position.X, position.Y, position.Z);
	int offset = index * m_chunk_data_size;
	DBG_ASSERT(index >= 0 && index < m_chunk_count);

	// Flag chunk as existing
	m_chunk_exists[index] = true;

	// Write in the voxel data.
	m_stream->Seek(m_voxel_data_offset + offset);
	m_stream->Write((char*)chunk->Get_Voxel_Buffer(), 0, m_chunk_data_size);

	// Flush header and data to disk.
	Flush();
}

void RegionFile::Load_Chunk(Chunk* chunk)
{
	MutexLock lock(m_access_mutex);
	
	IntVector3 position = chunk->Get_Position() - m_chunk_origin;
	int index = Flatten_Index(position.X, position.Y, position.Z);
	int offset = index * m_chunk_data_size;
	DBG_ASSERT(index >= 0 && index < m_chunk_count);

	// Write in the voxel data.
	m_stream->Seek(m_voxel_data_offset + offset);
	m_stream->Read((char*)chunk->Get_Voxel_Buffer(), 0, m_chunk_data_size);
}