// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_ELEMENTS_UIPANEL_
#define _ENGINE_UI_ELEMENTS_UIPANEL_

#include "Engine\UI\UIElement.h"
#include "Engine\UI\UIFrame.h"
#include "Engine\Renderer\Text\Font.h"
#include "Engine\Renderer\Text\FontRenderer.h"

#include "Engine\Renderer\Atlases\AtlasRenderer.h"

#include "Engine\Renderer\PrimitiveRenderer.h"

class AtlasHandle;
class UIManager;
class AtlasFrame;

class UIPanel : public UIElement 
{
protected:	
	friend class UILayout;

	Color m_backcolor;

	PrimitiveRenderer m_primitive_renderer;

public:
	UIPanel();
	~UIPanel();
	
	void Refresh();
	void After_Refresh();

	void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);

};

#endif

