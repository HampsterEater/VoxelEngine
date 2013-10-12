// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Text\FontFactory.h"
#include "Engine\Renderer\Text\FreeType\FreeType_Font.h"
#include "Engine\Renderer\Text\FreeType\FreeType_FontFactory.h"
#include "Engine\Renderer\Text\FreeType\FreeType_ResourceCompiler.h"

#include "Engine\IO\StreamFactory.h"

FT_Library FreeType_FontFactory::m_library;

FreeType_FontFactory::FreeType_FontFactory()
{
}

FreeType_FontFactory::~FreeType_FontFactory()
{
}

FT_Library FreeType_FontFactory::Get_FreeType_Library()
{
	if (m_library == NULL)
	{
		Init_FreeType();
	}
	return m_library;
}

void FreeType_FontFactory::Init_FreeType()
{
	DBG_LOG("Initializing FreeType font library ..");

	int result = FT_Init_FreeType(&m_library);
	DBG_ASSERT_STR(result == 0, "FreeType failed to initialize with error 0x%08x", result);
}

Font* FreeType_FontFactory::Try_Load(const char* url, FontFlags::Type flags)
{	
	// Compile the resource file.
	FreeType_ResourceCompiler compiler(url);
	if (compiler.Should_Compile())
	{
		compiler.Compile();
	}

	// Load configuration from the compiled path.
	std::string compiled_path = compiler.Get_Compiled_Path();

	// TODO: Load from compiled XML.

	return NULL;

	/*
	// Load in the font file.
	int size = stream->Length();
	char* buffer = new char[size];
	if (buffer == NULL)
	{
		SAFE_DELETE(stream);
		return NULL;
	}
	stream->Read(buffer, 0, size);

	// Load in font face.
	FT_Face face;
	int result = FT_New_Memory_Face(m_library, (FT_Byte*)buffer, size, 0, &face);
	if (result != 0)
	{
		DBG_LOG("Failed to open freetype font face: %s", url);
		SAFE_DELETE(buffer);
		SAFE_DELETE(stream);
		return NULL;
	}

	// Return resulting font!
	SAFE_DELETE(stream);
	return new FreeType_Font(m_library, face, buffer);
	*/
}