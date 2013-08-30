// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_TEXTUREFACTORY_
#define _ENGINE_RENDERER_TEXTURES_TEXTUREFACTORY_

#include "Generic\Types\LinkedList.h"
#include "Engine\IO\Stream.h"

class Texture;

class TextureFactory
{
private:
	static LinkedList<TextureFactory*> m_factories;

public:
		
	// Static methods.
	static Texture* Load(const char* url);

	// Constructors
	TextureFactory();	
	~TextureFactory();	

	// Derived factory methods.
	virtual Texture* Try_Load(const char* url) = 0;

};

#endif

