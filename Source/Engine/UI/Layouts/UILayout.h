// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_LAYOUTS_UILAYOUT_
#define _ENGINE_UI_LAYOUTS_UILAYOUT_

#include "Engine\Engine\FrameTime.h"

#include "Engine\Config\ConfigFile.h"

#include <vector>

class UIManager;
class UIScene;
class UIElement;
class UILayoutFactory;

struct UILayoutElementDefinition
{
public:
	virtual ~UILayoutElementDefinition();

	std::string								Type;
	std::vector<UILayoutElementDefinition*> Children;
};

// Define the derived property classes.
#define ELEMENT_START(name, classname) \
		class UILayoutElementDefinition_##classname : public UILayoutElementDefinition \
		{ \
			public: 
#define ELEMENT_END() \
		};
#define ELEMENT_PROPERTY(name, fieldname, type, required) \
		type fieldname; \
		bool fieldname##_exists;
#define ELEMENT_PROPERTY_ENUM_START(name, fieldname, type, required) \
		type::Type fieldname; \
		bool fieldname##_exists;
#define ELEMENT_PROPERTY_ENUM_END()
#define ELEMENT_ENUM_ENTRY(type, name) 
#define ELEMENT_ALLOW_CHILD(x)
#define ELEMENT_ALLOW_ANY_CHILD()
#define ELEMENT_FORCE_MAX_CHILDREN(x)
#define ELEMENT_MAX_CHILDREN(x)

#include "Engine\UI\Elements\UIElementDefinitions.inc"

#undef ELEMENT_ALLOW_CHILD
#undef ELEMENT_ALLOW_ANY_CHILD
#undef ELEMENT_FORCE_MAX_CHILDREN
#undef ELEMENT_MAX_CHILDREN
#undef ELEMENT_ENUM_ENTRY
#undef ELEMENT_PROPERTY_ENUM_START
#undef ELEMENT_PROPERTY_ENUM_END
#undef ELEMENT_PROPERTY
#undef ELEMENT_END
#undef ELEMENT_START

class UILayout
{
protected:
	friend class UIScene;
	friend class UILayoutFactory;

	UILayoutElementDefinition* m_root_element;

	UIElement* Instantiate_Element(const UILayoutElementDefinition* def) const;
	std::vector<UIElement*> Instantiate() const;

	bool Load_Element(ConfigFile* file, ConfigFileNode root_node, UILayoutElementDefinition*& result);
	bool Load_Elements(ConfigFile* file, UILayoutElementDefinition* root, std::vector<ConfigFileNode> children);
	static UILayout* Load(const char* url);

public:
	UILayout();
	~UILayout();

};

#endif

