// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\PrimitiveRenderer.h"
#include "Engine\Renderer\RenderPipeline.h"
#include "Engine\Renderer\Shaders\ShaderProgram.h"

#include "Generic\Math\Math.h"

PrimitiveRenderer::PrimitiveRenderer()
{
}

void PrimitiveRenderer::Apply_Primitive_Shader(const FrameTime& time, Color color)
{
	Renderer*				renderer	= Renderer::Get();
	RenderPipeline*			pipeline	= RenderPipeline::Get();
	RenderPipeline_Shader*	shader		= pipeline->Get_Shader_From_Name("solid_quad");
	
	pipeline->Apply_Shader(time, shader); 
	pipeline->Update_Shader_Uniforms();
	
	shader->Shader_Program->Bind_Vector("g_color", color.To_Vector4());	
}

void PrimitiveRenderer::Draw_Solid_Quad(const FrameTime& time, Rect xy, Color color)
{
	Renderer* renderer = Renderer::Get();
	Apply_Primitive_Shader(time, color);

	renderer->Draw_Quad(xy, Rect(0,0,1,1));
}

void PrimitiveRenderer::Draw_Line(const FrameTime& time, Vector3 start_zyz, Vector3 end_xyz, float size, Color color)
{
	Renderer* renderer = Renderer::Get();
	Apply_Primitive_Shader(time, color);

	renderer->Draw_Line(start_zyz.X, start_zyz.Y, start_zyz.Z, end_xyz.X, end_xyz.Y, end_xyz.Z, size);
}

void PrimitiveRenderer::Draw_Cube(const FrameTime& time, Vector3 size, Color color)
{
	Renderer* renderer = Renderer::Get();
	Apply_Primitive_Shader(time, color);

	Vector3 p1(0,		0,		size.Z);
	Vector3 p2(size.X,	0,		size.Z);
	Vector3 p3(size.X,	size.Y, size.Z);
	Vector3 p4(0,		size.Y, size.Z);
	Vector3 p5(size.X,	0,		0);
	Vector3 p6(0,		0,		0);
	Vector3 p7(0,		size.Y, 0);
	Vector3 p8(size.X,	size.Y, 0);

#define QUAD(x, y, z, w) renderer->Draw_Quad(x, y, z, w);
	QUAD(p1, p2, p3, p4);
	QUAD(p5, p6, p7, p8);
	QUAD(p2, p5, p8, p3);
	QUAD(p6, p1, p4, p7);
	QUAD(p4, p3, p8, p7);
	QUAD(p6, p5, p2, p1);
#undef QUAD
}

void PrimitiveRenderer::Draw_Wireframe_Cube(const FrameTime& time, Vector3 size, float line_size, Color color)
{
	Renderer* renderer = Renderer::Get();
	Apply_Primitive_Shader(time, color);

	float w = size.X;
	float h = size.Y;
	float d = size.Z;

	// Top
	renderer->Draw_Line(0, 0, 0,   w, 0, 0, line_size);
	renderer->Draw_Line(0, 0, 0,   0, 0, d, line_size);
	renderer->Draw_Line(0, 0, d,   w, 0, d, line_size);
	renderer->Draw_Line(w, 0, 0,   w, 0, d, line_size);

	// Bottom
	renderer->Draw_Line(0, h, 0,   w, h, 0, line_size);
	renderer->Draw_Line(0, h, 0,   0, h, d, line_size);
	renderer->Draw_Line(0, h, d,   w, h, d, line_size);
	renderer->Draw_Line(w, h, 0,   w, h, d, line_size);

	// Connectors.
	renderer->Draw_Line(0, 0, 0,   0, h, 0, line_size);
	renderer->Draw_Line(0, 0, d,   0, h, d, line_size);
	renderer->Draw_Line(w, 0, d,   w, h, d, line_size);
	renderer->Draw_Line(w, 0, 0,   w, h, 0, line_size);
}

void PrimitiveRenderer::Draw_Wireframe_Sphere(const FrameTime& time, float r, float line_size, Color color)
{
	Renderer* renderer = Renderer::Get();
	Apply_Primitive_Shader(time, color);

	int lats  = 12;
	int longs = 12;

	for (int i = 0; i <= lats; i++) 
	{
		double lat0 = PI * (-0.5 + (double) (i - 1) / lats);
		double z0   = sin(lat0);
		double zr0  =  cos(lat0);
    
		double lat1 = PI * (-0.5 + (double) i / lats);
		double z1   = sin(lat1);
		double zr1  = cos(lat1);
    
		for (int j = 0; j <= longs; j++) 
		{
			double lng = 2 * PI * (double) (j - 1) / longs;
			double x = cos(lng);
			double y = sin(lng);
    	
			renderer->Draw_Line
			(
				(x * zr0) * r, (y * zr0) * r, z0 * r,
				(x * zr1) * r, (y * zr1) * r, z1 * r,
				line_size
			);
		}
	}
}

void PrimitiveRenderer::Draw_Arrow(const FrameTime& time, Vector3 direction, float line_length, float pip_length, float line_size, Color color)
{
	Renderer* renderer = Renderer::Get();
	Apply_Primitive_Shader(time, color);

	Vector3 start = Vector3(0.0f, 0.0f, 0.0f); 
	Vector3 end   = direction * line_length;

	renderer->Draw_Line(start.X, start.Y, start.Z, end.X, end.Y, end.Z, line_size);

	// Draw the end "pips", I'm sure there is a far better way to do this,
	// but fuck maths. This is quick and it gets the job down for now.
	const int pip_count = 4;
	Vector3 pip_directions[pip_count] =
	{
		start.Direction().Cross(direction),
		-start.Direction().Cross(direction),
		start.Direction().Cross(direction).Cross(direction),
		-start.Direction().Cross(direction).Cross(direction),	
	};

	for (int i = 0; i < pip_count; i++)
	{
		Vector3 end_pip_dir = pip_directions[i];
		Vector3 end_pip		= (end + (end_pip_dir * pip_length)) - (direction * pip_length);
		renderer->Draw_Line(end.X, end.Y, end.Z, end_pip.X, end_pip.Y, end_pip.Z, line_size);
	}
}
