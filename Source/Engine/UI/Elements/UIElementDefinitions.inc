// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

// Eeehhhhhhhhhhhhhhh, this whole file is pretty dam fugly. Need to figure
// out a better method of doing this while restricting manual work.

#ifndef ELEMENT_START
#error "ELEMENT_START must be defined when including this file!"
#endif
#ifndef ELEMENT_END
#error "ELEMENT_END must be defined when including this file!"
#endif
#ifndef ELEMENT_END
#error "ELEMENT_PROPERTY must be defined when including this file!"
#endif
#ifndef ELEMENT_PROPERTY_ENUM_START
#error "ELEMENT_PROPERTY_ENUM_START must be defined when including this file!"
#endif
#ifndef ELEMENT_PROPERTY_ENUM_END
#error "ELEMENT_PROPERTY_ENUM_END must be defined when including this file!"
#endif
#ifndef ELEMENT_ENUM_ENTRY
#error "ELEMENT_ENUM_ENTRY must be defined when including this file!"
#endif

#ifndef ELEMENT_ALLOW_CHILD
#error "ELEMENT_ALLOW_CHILD must be defined when including this file!"
#endif
#ifndef ELEMENT_ALLOW_ANY_CHILD
#error "ELEMENT_ALLOW_ANY_CHILD must be defined when including this file!"
#endif
#ifndef ELEMENT_FORCE_MAX_CHILDREN
#error "ELEMENT_FORCE_MAX_CHILDREN must be defined when including this file!"
#endif
#ifndef ELEMENT_MAX_CHILDREN
#error "ELEMENT_MAX_CHILDREN must be defined when including this file!"
#endif

// Define Definitions:
//
// ELEMENT_START(element-name, element-class-name)
//	ELEMENT_PROPERTY(property-name, property-field-name, property-type, is-required)
// ELEMENT_END()
//
// These define all the different properties of elements that can be
// loaded and saved from layout files.

// ===================================================================
// Menu Elements
// ===================================================================
#include "Engine\UI\Elements\UIMenuBar.h"
#include "Engine\UI\Elements\UIMenuItem.h"

ELEMENT_START(MenuBar, UIMenuBar)
	ELEMENT_PROPERTY(Box,			m_box,			Rect,			true);
	ELEMENT_PROPERTY(Name,			m_name,			std::string,	true);
	ELEMENT_PROPERTY(ItemPadding,	m_item_padding,	Rect,			false);

	ELEMENT_ALLOW_CHILD				(UIMenuItem);
	ELEMENT_FORCE_MAX_CHILDREN		(false);
	ELEMENT_MAX_CHILDREN			(-1);
ELEMENT_END()

ELEMENT_START(MenuItem, UIMenuItem)
	ELEMENT_PROPERTY(Name,			m_name,			std::string,	true);
	ELEMENT_PROPERTY(Value,			m_value,		std::string,	true);
	ELEMENT_PROPERTY(Icon,			m_icon_name,	std::string,	false);

	ELEMENT_ALLOW_CHILD				(UIMenuItem);
	ELEMENT_FORCE_MAX_CHILDREN		(false);
	ELEMENT_MAX_CHILDREN			(-1);
ELEMENT_END()

// ===================================================================
// Text Elements
// ===================================================================
#include "Engine\UI\Elements\UILabel.h"

ELEMENT_START(Label, UILabel)
	ELEMENT_PROPERTY			(Box,			m_box,				Rect,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY 			(Value,			m_value,			std::string,			true);
	ELEMENT_PROPERTY			(Padding,		m_padding,			Rect,					false);
	ELEMENT_PROPERTY			(ForeColor,		m_color,			Color,					false);
	
	ELEMENT_PROPERTY_ENUM_START	(HAlign,		m_halign,			TextAlignment,			false);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Left);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Right);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Center);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Justified);
	ELEMENT_PROPERTY_ENUM_END	();	
	
	ELEMENT_PROPERTY_ENUM_START	(VAlign,		m_valign,			TextAlignment,			false);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Top);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Bottom);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Center);
		ELEMENT_ENUM_ENTRY		(TextAlignment, Justified);
	ELEMENT_PROPERTY_ENUM_END	();	

	ELEMENT_MAX_CHILDREN		(0);
	ELEMENT_FORCE_MAX_CHILDREN	(false);
ELEMENT_END()

// ===================================================================
// Container Elements
// ===================================================================
#include "Engine\UI\Elements\UISplitContainer.h"
#include "Engine\UI\Elements\UISplitContainerItem.h"

ELEMENT_START(SplitContainer, UISplitContainer)
	ELEMENT_PROPERTY			(Box,			m_box,				Rect,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(Resizable,		m_resizable,		bool,					true);
	
	ELEMENT_PROPERTY_ENUM_START	(Direction,		m_direction,		SplitContainerDirection,true);
		ELEMENT_ENUM_ENTRY		(SplitContainerDirection, Horizontal);
		ELEMENT_ENUM_ENTRY		(SplitContainerDirection, Vertical);
	ELEMENT_PROPERTY_ENUM_END	();	
	
	ELEMENT_ALLOW_CHILD			(UISplitContainerItem);
	ELEMENT_FORCE_MAX_CHILDREN	(true);
	ELEMENT_MAX_CHILDREN		(2);
ELEMENT_END()

ELEMENT_START(SplitContainerItem, UISplitContainerItem)
	ELEMENT_PROPERTY			(Size,			m_size,				float,					false);
	ELEMENT_PROPERTY			(Fixed,			m_fixed,			bool,					true);

	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// Panel Elements
// ===================================================================
#include "Engine\UI\Elements\UIPanel.h"

ELEMENT_START(Panel, UIPanel)
	ELEMENT_PROPERTY			(Box,			m_box,				Rect,					true);
	ELEMENT_PROPERTY			(Name,			m_name,				std::string,			true);
	ELEMENT_PROPERTY			(BackColor,		m_backcolor,		Color,					false);
	
	ELEMENT_ALLOW_ANY_CHILD		();
	ELEMENT_FORCE_MAX_CHILDREN	(false);
	ELEMENT_MAX_CHILDREN		(-1);
ELEMENT_END()

// ===================================================================
// Toolbar Elements
// ===================================================================
#include "Engine\UI\Elements\UIToolbar.h"
#include "Engine\UI\Elements\UIToolbarItem.h"

ELEMENT_START(Toolbar, UIToolbar)
	ELEMENT_PROPERTY(Box,			m_box,			Rect,			true);
	ELEMENT_PROPERTY(Name,			m_name,			std::string,	true);
	ELEMENT_PROPERTY(ItemPadding,	m_item_padding,	Rect,			false);

	ELEMENT_ALLOW_CHILD				(UIToolbarItem);
	ELEMENT_FORCE_MAX_CHILDREN		(false);
	ELEMENT_MAX_CHILDREN			(-1);
ELEMENT_END()

ELEMENT_START(ToolbarItem, UIToolbarItem)
	ELEMENT_PROPERTY(Name,			m_name,			std::string,	true);
	ELEMENT_PROPERTY(Icon,			m_icon_name,	std::string,	true);

	ELEMENT_FORCE_MAX_CHILDREN		(true);
	ELEMENT_MAX_CHILDREN			(0);
ELEMENT_END()