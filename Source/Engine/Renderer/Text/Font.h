// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FONT_
#define _ENGINE_RENDERER_TEXT_FONT_

#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Point.h"

class Texture;

struct FontGlyph
{
	unsigned int Glyph;
	Rect		 UV;
	Texture*	 Texture;
	Point		 Size;
	Point		 Offset;
	Point		 Advance;
	float		 Baseline;
};

struct FontFlags
{
	enum Type
	{
		None = 0
	};
};

class Font
{
private:
	
protected:

public:

	// Destructor!
	virtual ~Font();

	// Actual font stuff!
	virtual float Get_SDF_Spread() = 0;	
	virtual float Get_SDF_Source_Size() = 0;
	virtual float Get_Baseline() = 0;

	virtual FontGlyph Get_Glyph(unsigned int character) = 0;
	virtual Point	  Get_Kerning(unsigned int prev, unsigned int next) = 0;

};

#endif

