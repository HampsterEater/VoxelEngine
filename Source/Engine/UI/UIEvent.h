// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIEVENT_
#define _ENGINE_UI_UIEVENT_

#include "Engine\Engine\FrameTime.h"
#include "Generic\Types\Rectangle.h"

#include <vector>

class UIManager;
class UIScene;
class UILayout;
class UIElement;

struct UIEventType
{
	enum Type
	{
#define EVENT(name) name,
#include "Engine\UI\UIEventList.inc"
#undef EVENT
	};
};

struct UIEvent
{	
	static const char* Event_Names[];
	
	UIEventType::Type Type;
	const char*		  Name;
	UIElement*		  Source;

	UIEvent(UIEventType::Type type, UIElement* source);
};

#endif

