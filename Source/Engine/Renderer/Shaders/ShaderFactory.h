// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_SHADERS_SHADERFACTORY_
#define _ENGINE_RENDERER_SHADERS_SHADERFACTORY_

#include "Generic\Types\LinkedList.h"
#include "Engine\IO\Stream.h"

#include "Engine\Renderer\Shaders\Shader.h"

class ShaderFactory
{
private:
	static LinkedList<ShaderFactory*> m_factories;

public:
		
	// Static methods.
	static Shader* Load				(const char* url, ShaderType::Type type);

	// Constructors
	ShaderFactory();	
	~ShaderFactory();	

	// Derived factory methods.
	virtual Shader* Try_Load(const char* url, ShaderType::Type type) = 0;

};

#endif

