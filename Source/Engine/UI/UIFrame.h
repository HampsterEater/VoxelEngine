// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIFRAME_
#define _ENGINE_UI_UIFRAME_

#include "Engine\Engine\FrameTime.h"
#include "Generic\Types\Rectangle.h"

#include "Engine\Renderer\Atlases\AtlasRenderer.h"

#include <vector>

class UIManager;
class AtlasHandle;

class UIFrame
{
protected:
	AtlasFrame* m_frames[9];

public:
	UIFrame();
	UIFrame(AtlasHandle* atlas, std::string name_pattern);

	void Draw_Frame(const FrameTime& time, UIManager* manager, AtlasRenderer& renderer, Rect rect);

};

#endif

