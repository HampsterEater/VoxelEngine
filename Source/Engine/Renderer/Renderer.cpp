// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Renderer.h"

#include "Generic\Patterns\Singleton.h"
#include "Generic\Math\Math.h"

#ifdef PLATFORM_WIN32
#include "Engine\Renderer\OpenGL\OpenGL_Renderer.h"
#endif

Renderer* Renderer::Create()
{
#ifdef PLATFORM_WIN32
	return new OpenGL_Renderer();
#else
	#error "Platform unsupported."
#endif
}

/*
void Renderer::Draw_Wireframe_Cube(float w, float h, float d)
{
	// Top
	Draw_Line(0, 0, 0,   w, 0, 0);
	Draw_Line(0, 0, 0,   0, 0, d);
	Draw_Line(0, 0, d,   w, 0, d);
	Draw_Line(w, 0, 0,   w, 0, d);

	// Bottom
	Draw_Line(0, h, 0,   w, h, 0);
	Draw_Line(0, h, 0,   0, h, d);
	Draw_Line(0, h, d,   w, h, d);
	Draw_Line(w, h, 0,   w, h, d);

	// Connectors.
	Draw_Line(0, 0, 0,   0, h, 0);
	Draw_Line(0, 0, d,   0, h, d);
	Draw_Line(w, 0, d,   w, h, d);
	Draw_Line(w, 0, 0,   w, h, 0);
}

void Renderer::Draw_Cube(float w, float h, float d)
{
	// Top
	Draw_Line(0, 0, 0,   w, 0, 0);
	Draw_Line(0, 0, 0,   0, 0, d);
	Draw_Line(0, 0, d,   w, 0, d);
	Draw_Line(w, 0, 0,   w, 0, d);

	// Bottom
	Draw_Line(0, h, 0,   w, h, 0);
	Draw_Line(0, h, 0,   0, h, d);
	Draw_Line(0, h, d,   w, h, d);
	Draw_Line(w, h, 0,   w, h, d);

	// Connectors.
	Draw_Line(0, 0, 0,   0, h, 0);
	Draw_Line(0, 0, d,   0, h, d);
	Draw_Line(w, 0, d,   w, h, d);
	Draw_Line(w, 0, 0,   w, h, 0);
}

void Renderer::Draw_Wireframe_Sphere(float r)
{
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
    	
			Draw_Line(
						(x * zr0) * r, (y * zr0) * r, z0 * r,
						(x * zr1) * r, (y * zr1) * r, z1 * r
					);
		}
	}
}

void Renderer::Draw_Arrow(Vector3 direction, float line_length, float pip_length)
{
	Vector3 start = Vector3(0.0f, 0.0f, 0.0f); 
	Vector3 end   = direction * line_length;

	Draw_Line(start.X, start.Y, start.Z, end.X, end.Y, end.Z, 4.0f);

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
		Draw_Line(end.X, end.Y, end.Z, end_pip.X, end_pip.Y, end_pip.Z, 8.0f);
	}
}
*/