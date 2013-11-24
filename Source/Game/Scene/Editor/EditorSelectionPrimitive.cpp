// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game\Scene\Editor\EditorSelectionPrimitive.h"

#include "Engine\Renderer\PrimitiveRenderer.h"

EditorSelectionPrimitive::EditorSelectionPrimitive()
	: m_size(0.1f, 0.1f, 0.1f)
{
}

void EditorSelectionPrimitive::Set_Size(Vector3 size)
{
	m_size = size;
}

Vector3 EditorSelectionPrimitive::Get_Size()
{
	return m_size;
}

void EditorSelectionPrimitive::Tick(const FrameTime& time)
{
}

void EditorSelectionPrimitive::Draw(const FrameTime& time, RenderPipeline* pipeline)
{
	Renderer* renderer = Renderer::Get();
	PrimitiveRenderer primitive_renderer;

	renderer->Set_World_Matrix(Matrix4::Translate(m_position) * Matrix4::Rotate(m_rotation));
	pipeline->Update_Shader_Uniforms();	
	primitive_renderer.Draw_Wireframe_Cube(time, m_size, 2.0f, Color::Blue);
}
