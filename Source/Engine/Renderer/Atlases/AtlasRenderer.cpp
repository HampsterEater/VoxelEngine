// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Atlases\AtlasRenderer.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\Atlas.h"

#include "Engine\Renderer\RenderPipeline.h"

#include "Engine\Scene\Camera.h"

#include "Engine\Renderer\Textures\Texture.h"
#include "Engine\Renderer\Textures\TextureFactory.h"

AtlasRenderer::AtlasRenderer(AtlasHandle* atlas)
	: m_atlas(atlas)
{
}

AtlasRenderer::AtlasRenderer()
	: m_atlas(NULL)
{
}

AtlasRenderer::~AtlasRenderer()
{
}

void AtlasRenderer::Draw_Frame(const FrameTime& time, AtlasFrame* frame, Rect  point, Color color)
{
	Renderer*				renderer	= Renderer::Get();
	RenderPipeline*			pipeline	= RenderPipeline::Get();
	Rect					viewport	= pipeline->Get_Active_Camera()->Get_Viewport();
	RenderPipeline_Shader*	shader		= pipeline->Get_Shader_From_Name("ui_element");

	// Initial rendering state for SDF.
	renderer->Set_Alpha_Test(false);
	renderer->Set_Blend(true);
	renderer->Set_Blend_Function(RendererOption::Src_Alpha_One_Minus_Src_Alpha);
	
	pipeline->Apply_Shader(time, shader); 
	pipeline->Update_Shader_Uniforms();
	shader->Shader_Program->Bind_Texture("g_texture", 0);
	shader->Shader_Program->Bind_Vector("g_color", color.To_Vector4());
	renderer->Bind_Texture(frame->Texture->Texture, 0);

	renderer->Draw_Quad(Rect(point.X - frame->Origin.X, point.Y - frame->Origin.Y, point.Width, point.Height), frame->UV);

	renderer->Set_Alpha_Test(true);
}

void AtlasRenderer::Draw_Frame(const FrameTime& time, AtlasFrame* frame, Point point, Color color)
{
	Draw_Frame(time, frame, Rect(point.X, point.Y, frame->Rectangle.Width, frame->Rectangle.Height), color);
}

void AtlasRenderer::Draw_Frame(const FrameTime& time, const char* frame, Point point, Color color)
{
	AtlasFrame* f = m_atlas->Get()->Get_Frame(frame);
	Draw_Frame(time, frame, Rect(point.X, point.Y, f->Rectangle.Width, f->Rectangle.Height), color);
}

void AtlasRenderer::Draw_Frame(const FrameTime& time, const char* frame, Rect  point, Color color)
{
	AtlasFrame* f = m_atlas->Get()->Get_Frame(frame);
	Draw_Frame(time, frame, point, color);
}
