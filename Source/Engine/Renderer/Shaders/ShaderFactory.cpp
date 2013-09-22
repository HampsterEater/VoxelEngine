// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Shaders\ShaderFactory.h"
#include "Engine\Renderer\Shaders\Shader.h"

LinkedList<ShaderFactory*> ShaderFactory::m_factories;

Shader* ShaderFactory::Load(const char* url, ShaderType::Type type)
{
	for (LinkedList<ShaderFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		ShaderFactory* factory = *iter;
		Shader* result = factory->Try_Load(url, type);
		if (result != NULL)
		{
			DBG_LOG("Loaded shader: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to load shader: %s", url);
	return NULL;
}

ShaderFactory::ShaderFactory()
{
	m_factories.Add(this);
}

ShaderFactory::~ShaderFactory()
{
	m_factories.Remove(m_factories.Find(this));
}
