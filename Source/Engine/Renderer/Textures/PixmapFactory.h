// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_PIXMAPFACTORY_
#define _ENGINE_RENDERER_TEXTURES_PIXMAPFACTORY_

#include "Generic\Types\LinkedList.h"
#include "Generic\Types\HashTable.h"
#include "Engine\IO\Stream.h"

#include "Engine\Engine\FrameTime.h"

#include "Engine\Renderer\Textures\Pixmap.h"

class PixmapFactory
{
private:
	static LinkedList<PixmapFactory*> m_factories;

public:
		
	// Static methods.
	static bool	   Save	(const char* url, Pixmap* texture);
	static Pixmap* Load	(const char* url);
	
	// Constructors
	PixmapFactory();	
	virtual ~PixmapFactory();	

	// Derived factory methods.
	virtual Pixmap*  Try_Load(const char* url) = 0;
	virtual bool     Try_Save(const char* url, Pixmap* texture) = 0;

};

#endif

