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

	// Load compiled path.
	return compiler.Load_Compiled();
}