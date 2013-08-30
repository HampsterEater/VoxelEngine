// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_OPENGL_OPENGL_RENDERER_
#define _ENGINE_RENDERER_OPENGL_OPENGL_RENDERER_

#include "Generic\Patterns\Singleton.h"
#include "Engine\Renderer\Renderer.h"

#ifdef PLATFORM_WIN32
#include <Windows.h>
#include "Engine\Display\Win32\Win32_Display.h"
#endif

#include <vector>

class Display;
class OpenGL_Texture;

// If this is not defined then display lists will be used.
//#define USE_VBO_FOR_MESH

// Maximum number of meshs that can exist at one time. Theoretical max of this is load_distance_w*load_distance_h*load_Distance_d
#define MAX_MESHS			1024 * 256

// Opengl rendering fun times!
class OpenGL_Renderer : public Renderer
{
private:
	struct Vertex
	{
		Vector3 position;
		Vector3 normal;
		float r, g, b, a;			// Not actually used anymore! Color=Slow
		float u, v;
	};

	struct Triangle
	{
		int vertex_1;
		int vertex_2;
		int vertex_3;
	};

	struct Mesh
	{
		bool		active;

#ifdef USE_VBO_FOR_MESH
		GLuint		vbo_vertices_buffer;
		GLuint		vbo_texcoords_buffer;
		GLuint		vbo_normals_buffer;
		float*		vbo_vertices;
		float*		vbo_texcoords;
		float*		vbo_normals;
		int			vbo_vertices_count;
#else
		GLuint		list_id;
#endif

		Vertex*		vertices;
		Triangle*	triangles;

		int			vertex_counter;
		int			vertex_count;
		int			triangle_counter;
		int			triangle_count;
	};

	Mesh				m_meshs[MAX_MESHS];
	Display*			m_display;
	Camera*				m_active_camera;
	bool				m_initialized;
	
	float				m_near_clip;
	float				m_far_clip;

	OpenGL_Texture*		m_binded_texture;

	friend class Renderer;

	OpenGL_Renderer();
	~OpenGL_Renderer();

	// Extensions!
	void Initialize_OpenGL();
	bool Is_Extension_Supported(const char* name);

	// Mesh stuff
	int Get_Free_Mesh_Index();

public:
	
	// Base functions.	
	void Tick(const FrameTime& time);
	void Draw(const FrameTime& time);

	// Display related settings.
	bool Set_Display(Display* display);
	Camera* Get_Active_Camera();
	
	// Visibility checking.
	Frustum Get_Frustum();
	
	// Rendering.		
	void Bind_Texture(Texture* texture);
	Texture* Create_Texture(char* data, int width, int height, int pitch, TextureFormat::Type format);

	void Push_Matrix();
	void Pop_Matrix();
	void Translate_World_Matrix(float x, float y, float z);
	void Rotate_World_Matrix(float x, float y, float z);

	void Render_Mesh(int id);
	void Destroy_Mesh(int id);
	int  Start_Mesh(int vertices, int triangles);
	void End_Mesh(int id);
	int  Add_Mesh_Vertex(int id, Vector3 position, Vector3 normal, float r, float g, float b, float a, float u, float v);
	int  Add_Mesh_Triangle(int id, int vertex1, int vertex2, int vertex3);

	// Immediate rendering.	
	void Draw_Wireframe_Cube(float w, float h, float d);
	void Draw_Wireframe_Sphere(float r);
	void Draw_Line(float x1, float y1, float z1, float x2, float y2, float z2);
};

#endif

