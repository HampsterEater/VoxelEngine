// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_TEXTUREFACTORY_
#define _ENGINE_RENDERER_TEXTURES_TEXTUREFACTORY_

#include "Generic\Types\LinkedList.h"
#include "Generic\Types\HashTable.h"
#include "Engine\IO\Stream.h"

#include "Engine\Engine\FrameTime.h"

#include "Engine\Renderer\Textures\Texture.h"
#include "Engine\Renderer\Textures\TextureHandle.h"

class TextureFactory
{
private:
	static LinkedList<TextureFactory*>		m_factories;
	static HashTable<TextureHandle*, int>	m_loaded_textures;

public:
		
	// Static methods.
	static bool			  Save				 (const char* url, Texture* texture, TextureFlags::Type flags);
	static TextureHandle* Load				 (const char* url, TextureFlags::Type flags);
	static Texture*		  Load_Without_Handle(const char* url, TextureFlags::Type flags);
	
	// Constructors
	TextureFactory();	
	virtual ~TextureFactory();	

	// Derived factory methods.
	virtual Texture* Try_Load(const char* url, TextureFlags::Type flags) = 0;
	virtual bool     Try_Save(const char* url, Texture* texture, TextureFlags::Type flags) = 0;

};

#endif

