// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Audio\FMod\FMod_AudioRenderer.h"
#include "Engine\Engine\GameEngine.h"

#include "Engine\IO\StreamFactory.h"
#include "Engine\IO\Stream.h"

#include "Generic\Math\Math.h"

FMOD_RESULT F_CALLBACK FMod_AudioRenderer::fmod_open(const char* name, int unicode, unsigned int* filesize, void** handle, void** userdata)
{
	DBG_ASSERT(unicode == false);

	Stream* stream = StreamFactory::Open(name, StreamMode::Read);
	if (stream == NULL)
	{
		return FMOD_ERR_FILE_NOTFOUND;
	}

	*filesize = stream->Length(); 
	*handle   = stream;

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMod_AudioRenderer::fmod_close(void* handle, void* userdata)
{
	Stream* stream = static_cast<Stream*>(handle);
	if (stream == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

	stream->Close();
	delete stream;

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMod_AudioRenderer::fmod_seek(void* handle, unsigned int pos, void* userdata)
{
	Stream* stream = static_cast<Stream*>(handle);
	if (stream == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

	stream->Seek(pos);
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMod_AudioRenderer::fmod_read(void* handle, void* buffer, unsigned int sizebytes, unsigned int* bytesread, void* userdata)
{
	Stream* stream = static_cast<Stream*>(handle);
	if (stream == NULL)
	{
		return FMOD_ERR_INVALID_PARAM;
	}

	unsigned int can_read = Min(stream->Length() - stream->Position(), sizebytes);
	*bytesread = can_read;

	if (can_read > 0)
	{
		stream->Read((char*)buffer, 0, can_read);
		return FMOD_OK;
	}

	return can_read < sizebytes ? FMOD_ERR_FILE_EOF : FMOD_OK;
}

FMod_AudioRenderer::FMod_AudioRenderer()
{
	const GameEngineConfig* config = GameEngine::Get()->Get_Config();
	int max_channels = config->audio_max_channels;

	DBG_LOG("Creating fmod audio system ...");
	{
		FMOD_RESULT result = FMOD_System_Create(&m_system);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_Create Failed - Result: 0x%08x", result);
	}
	
	DBG_LOG("Initializing fmod system ...");
	{
		FMOD_RESULT result = FMOD_System_Init(m_system, max_channels, FMOD_INIT_NORMAL, NULL);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_Init Failed - Result: 0x%08x", result);

		unsigned int version = 0;
		result = FMOD_System_GetVersion(m_system, &version);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetVersion Failed - Result: 0x%08x", result);
		
		int hardware_channels = 0;
		result = FMOD_System_GetHardwareChannels(m_system, &hardware_channels);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetHardwareChannels Failed - Result: 0x%08x", result);

		int driver_id = 0;
		result = FMOD_System_GetDriver(m_system, &driver_id);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetDriver Failed - Result: 0x%08x", result);

		char driver_name[1024];
		result = FMOD_System_GetDriverInfo(m_system, driver_id, driver_name, 1024, NULL);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetDriverInfo Failed - Result: 0x%08x", result);

		int sample_rate = 0;
		result = FMOD_System_GetSoftwareFormat(m_system, &sample_rate, NULL, NULL, NULL, NULL, NULL);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_GetSoftwareFormat Failed - Result: 0x%08x", result);

		DBG_LOG("  FMOD version       : %i.%i.%i (0x%08x)", (version >> 16) & 0xFFFF, (version >> 8) & 0xFF, version & 0xFF, version);
		DBG_LOG("  Built FMOD version : %i.%i.%i (0x%08x)", (FMOD_VERSION >> 16) & 0xFFFF, (FMOD_VERSION >> 8) & 0xFF, FMOD_VERSION & 0xFF, FMOD_VERSION);
		DBG_LOG("  Max channels       : %i", max_channels);
		DBG_LOG("  Hardware channels  : %i", hardware_channels);
		DBG_LOG("  Driver name        : %s", driver_name);
		DBG_LOG("  Sample rate        : %i", sample_rate);
	}

	DBG_LOG("Creating fmod file system ...");
	{
		FMOD_RESULT result = FMOD_System_SetFileSystem(m_system, fmod_open, fmod_close, fmod_read, fmod_seek, NULL, NULL, 2048);
		DBG_ASSERT_STR(result == FMOD_OK, "FMOD_System_SetFileSystem Failed - Result: 0x%08x", result);
	}
}

FMod_AudioRenderer::~FMod_AudioRenderer()
{
	if (m_system != NULL)
	{
		DBG_LOG("Releasing fmod system ...");
		{
			FMOD_RESULT result = FMOD_System_Release(m_system);
			DBG_ASSERT_STR(result == FMOD_OK, "Result: 0x%08x", result);

			m_system = NULL;
		}
	}
}

FMOD_SYSTEM* FMod_AudioRenderer::Get_FMod_System()
{
	return m_system;
}
	
void FMod_AudioRenderer::Tick(const FrameTime& time)
{
	FMOD_System_Update(m_system);
}