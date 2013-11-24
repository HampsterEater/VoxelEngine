// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Drawable.h"

#include "Generic\Helper\StringHelper.h"

Drawable::Drawable()
	: m_draw_camera(NULL)
	, m_render_slot_hash(0)
{
}

void Drawable::Set_Draw_Camera(Camera* camera)
{
	m_draw_camera = camera;
}

Camera* Drawable::Get_Draw_Camera()
{
	return m_draw_camera;
}

void Drawable::Set_Render_Slot(const char* name)
{
	m_render_slot_hash = StringHelper::Hash(name);
}

int	Drawable::Get_Render_Slot_Hash()
{
	return m_render_slot_hash;
}