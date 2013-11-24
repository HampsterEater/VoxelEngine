// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Atlases\AtlasFactory.h"
#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\AtlasResourceCompiler.h"

#include "Generic\Helper\StringHelper.h"

HashTable<AtlasHandle*, int>	AtlasFactory::m_loaded_atlases;

AtlasHandle* AtlasFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Atlas already loaded?
	if (m_loaded_atlases.Contains(url_hash))
	{
		DBG_LOG("Loaded atlas from cache: %s", url);
		return m_loaded_atlases.Get(url_hash);
	}

	// Try and load atlas!
	Atlas* atlas = Load_Without_Handle(url);
	if (atlas != NULL)
	{		
		AtlasHandle* handle = new AtlasHandle(url, atlas);

		m_loaded_atlases.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

Atlas* AtlasFactory::Load_Without_Handle(const char* url)
{
	Atlas* result = Load_Atlas(url);
	if (result != NULL)
	{
		DBG_LOG("Loaded atlas: %s", url);
		return result;
	}
	else
	{
		DBG_LOG("Failed to load atlas: %s", url);
		return NULL;
	}
}

Atlas* AtlasFactory::Load_Atlas(const char* url)
{
	// Compile the resource file.
	AtlasResourceCompiler compiler(url);
	if (compiler.Should_Compile())
	{
		compiler.Compile();
	}

	// Load compiled path.
	return compiler.Load_Compiled();
}

AtlasFactory::AtlasFactory()
{
}

