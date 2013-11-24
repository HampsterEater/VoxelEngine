// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\UI\UIEvent.h"

const char* UIEvent::Event_Names[] =
{
#define EVENT(name) #name,
#include "Engine\UI\UIEventList.inc"
#undef EVENT
};

UIEvent::UIEvent(UIEventType::Type type, UIElement* source)
	: Type(type)
	, Source(source)
{
	Name = Event_Names[(int)type];
}