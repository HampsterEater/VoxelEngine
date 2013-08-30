// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\OpenGL\OpenGL_Renderer.h"
#include "Engine\Display\Display.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Scene\Actor.h"
#include "Engine\Scene\Camera.h"
#include "Engine\Renderer\Drawable.h"
#include "Engine\Engine\GameEngine.h"

#include "Generic\Math\Math.h"
#include "Generic\Types\Matrix4.h"
#include "Generic\Types\Rectangle.h"

#include "Engine\Renderer\OpenGL\OpenGL_Texture.h"

#ifdef PLATFORM_WIN32
#include "Engine\Display\Win32\Win32_Display.h"
#endif

#include <vector>

// Extension methods to opengl that we need.
PFNGLGENBUFFERSARBPROC				glGenBuffers				= NULL;                  
PFNGLBINDBUFFERARBPROC				glBindBuffer				= NULL;                 
PFNGLBUFFERDATAARBPROC				glBufferData				= NULL;                
PFNGLDELETEBUFFERSARBPROC			glDeleteBuffers				= NULL;  

#ifdef PLATFORM_WIN32
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);              
PFNWGLSWAPINTERVALEXTPROC			wglSwapInterval				= NULL;  
#endif

OpenGL_Renderer::OpenGL_Renderer()
	: m_initialized(false)
	, m_near_clip(0.1f)
	, m_far_clip(100.0f)
	, m_active_camera(NULL)
	, m_display(NULL)
	, m_binded_texture(NULL)
{
}

OpenGL_Renderer::~OpenGL_Renderer()
{
	// check meshs were correctly destroyed!
	for (int i = 0; i < MAX_MESHS; i++)
	{
		if (m_meshs[i].active == true)
		{
			Destroy_Mesh(i);
		}
	}
}

void OpenGL_Renderer::Initialize_OpenGL()
{
	DBG_ASSERT(Is_Extension_Supported("GL_ARB_vertex_buffer_object"));

	glGenBuffers		= (PFNGLGENBUFFERSARBPROC)wglGetProcAddress("glGenBuffersARB");
	glBindBuffer		= (PFNGLBINDBUFFERARBPROC)wglGetProcAddress("glBindBufferARB");
	glBufferData		= (PFNGLBUFFERDATAARBPROC)wglGetProcAddress("glBufferDataARB");
	glDeleteBuffers		= (PFNGLDELETEBUFFERSARBPROC)wglGetProcAddress("glDeleteBuffersARB");
	wglSwapInterval		= (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	
	wglSwapInterval(0);

	for (int i = 0; i < MAX_MESHS; i++)
	{
		m_meshs[i].active = false;
	}

	// Clear screen.	
	glShadeModel(GL_SMOOTH);							
	glClearColor(125.0f / 255.0f, 154.0f / 255.0f, 234.0f / 255.0f, 0.5f);				
	glClearDepth(1.0f);									
	glEnable(GL_DEPTH_TEST);							
	glDepthFunc(GL_LEQUAL);								
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);	
	
	// Cheap shitty fog - todo:add to a Set_Fog etc call.
	float max_view_distance = 27.0f;
	GLfloat fogColor[] = { 125.0f / 255.0f, 154.0f / 255.0f, 234.0f / 255.0f, 0.5f };
	glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, max_view_distance * 0.75);
    glFogf(GL_FOG_END, max_view_distance);
	//glEnable(GL_FOG);
	
	// Wireframe
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	// Lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_COLOR_MATERIAL);
	
	GLfloat lightpos[] = { 0.f, 20.f, 0.f, 0.f };
	GLfloat lightambient[] = { 0.75f, 0.75f, 0.75f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightambient);
}

bool OpenGL_Renderer::Is_Extension_Supported(const char* szTargetExtension)
{
	// Nicked from NeHe, because I'm lazy as fuck.

    const unsigned char *pszExtensions = NULL;
    const unsigned char *pszStart;
    unsigned char *pszWhere, *pszTerminator;
 
    // Extension names should not have spaces
    pszWhere = (unsigned char *)strchr(szTargetExtension, ' ');
	if (pszWhere || *szTargetExtension == '\0')
	{
        return false;
	}

    // Get Extensions String
    pszExtensions = glGetString(GL_EXTENSIONS);
 
    // Search The Extensions String For An Exact Copy
    pszStart = pszExtensions;
    for (;;)
    {
        pszWhere = (unsigned char *)strstr((const char *)pszStart, szTargetExtension);
        if (!pszWhere)
		{
            break;
		}

		pszTerminator = pszWhere + strlen(szTargetExtension);
        if (pszWhere == pszStart || *( pszWhere - 1 ) == ' ')
		{
            if (*pszTerminator == ' ' || *pszTerminator == '\0')
			{
                return true;
			}
		}
        pszStart = pszTerminator;
    }

    return false;
}

int OpenGL_Renderer::Get_Free_Mesh_Index()
{
	for (int i = 0; i < MAX_MESHS; i++)
	{
		if (m_meshs[i].active == false)
		{
			return i;
		}
	}
	return -1;
}

void OpenGL_Renderer::Tick(const FrameTime& time)
{

}

void OpenGL_Renderer::Draw(const FrameTime& time)
{	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glViewport(0, 0, m_display->Get_Width(), m_display->Get_Height()); 
	
	// Render the scene for each camera.
	std::vector<Camera*>& cameras = GameEngine::Get()->Get_Scene()->Get_Cameras();
	for (auto iter = cameras.begin(); iter != cameras.end(); iter++)
	{
		Camera* camera = *iter;

		m_active_camera = camera;

		Rect	  viewport = camera->Get_Viewport();
		Vector3   rotation = camera->Get_Rotation();
		Vector3   position = camera->Get_Position();

		// Calculate projection matrix.
		//Matrix4 projection_matrix = Matrix4::Perspective(20, viewport.Width / viewport.Height, 5, 15);

		// Calculate view matrix.
		float horizontal = rotation.Y;
		float vertical   = rotation.Z;
		Vector3 direction
		(
			cos(vertical) * sin(horizontal),
			sin(vertical),
			cos(vertical) * cos(horizontal)
		);
		Vector3	right
		(
			sin(horizontal - 3.14f / 2.0f),
			0,
			cos(horizontal - 3.14f / 2.0f)
		);
		Vector3 center = position + direction;
		Vector3 up = right.Cross(direction);
		//Matrix4 view_matrix = Matrix4::LookAt(position, center, up);

		// Apply projection matrix.
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();	
		//glLoadMatrixf(reinterpret_cast<GLfloat*>(projection_matrix.Elements));		
		gluPerspective(camera->Get_FOV(), viewport.Width / (float) viewport.Height, m_near_clip, m_far_clip);
		glViewport((GLint)viewport.X, (GLint)viewport.Y, (GLsizei)viewport.Width, (GLsizei)viewport.Height);	

		// Apply view matrix.
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(position.X, position.Y, position.Z, center.X, center.Y, center.Z, up.X, up.Y, up.Z);
		//glLoadMatrixf(reinterpret_cast<GLfloat*>(view_matrix.Elements));
		
		// Render all drawables.
		std::vector<Drawable*>& drawable = GameEngine::Get()->Get_Scene()->Get_Drawables();
		for (auto iter = drawable.begin(); iter != drawable.end(); iter++)
		{
			Drawable* drawable = *iter;
			drawable->Draw(time, this);
		}
	}

	// Swap buffers.
	m_display->Swap_Buffers();
}

bool OpenGL_Renderer::Set_Display(Display* display)
{
	m_display = display;

	if (m_initialized == false)
	{
		Initialize_OpenGL();
		m_initialized = true;
	}

	return true;
}

Camera* OpenGL_Renderer::Get_Active_Camera()
{
	return m_active_camera;
}

Frustum OpenGL_Renderer::Get_Frustum()
{
	DBG_ASSERT(m_active_camera != NULL);
	
	Rect	  viewport = m_active_camera->Get_Viewport();
	Vector3   rotation = m_active_camera->Get_Rotation();
	Vector3   position = m_active_camera->Get_Position();

	// Calculate view matrix.
	float horizontal = rotation.Y;
	float vertical   = rotation.Z;
	Vector3 direction
	(
		cos(vertical) * sin(horizontal),
		sin(vertical),
		cos(vertical) * cos(horizontal)
	);
	Vector3	right
	(
		sin(horizontal - 3.14f / 2.0f),
		0,
		cos(horizontal - 3.14f / 2.0f)
	);
	Vector3 center = position + direction;
	Vector3 up = right.Cross(direction);
	
	return Frustum(m_active_camera->Get_FOV(),
				   viewport.Width / (float) viewport.Height,
				   m_near_clip,
				   m_far_clip,
				   position,
				   center,
				   up);
}

void OpenGL_Renderer::Bind_Texture(Texture* texture)
{
	OpenGL_Texture* glTexture = dynamic_cast<OpenGL_Texture*>(texture);

	if (m_binded_texture != glTexture)
	{
		glBindTexture(GL_TEXTURE_2D, glTexture->Get_ID());
		m_binded_texture = glTexture;
	}
}

Texture* OpenGL_Renderer::Create_Texture(char* data, int width, int height, int pitch, TextureFormat::Type format)
{
	GLuint texture_id;	

	// Generate and bind texture.
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	
	// And some nice filtering and clamp the texture.
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);

	// Upload data to GPU.
	switch (format)
	{
	case TextureFormat::R8G8B8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			break;
		}
	case TextureFormat::R8G8B8A8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			break;
		}
	}

	// Reset binding to previous texture.
	if (m_binded_texture != NULL)
	{
		Bind_Texture(m_binded_texture);
	}

	return new OpenGL_Texture(texture_id, data, width, height, pitch, format);
}

void OpenGL_Renderer::Push_Matrix()
{
	glPushMatrix();
}

void OpenGL_Renderer::Pop_Matrix()
{
	glPopMatrix();
}

void OpenGL_Renderer::Translate_World_Matrix(float x, float y, float z)
{
	glTranslatef(x, y, z);
}

void OpenGL_Renderer::Rotate_World_Matrix(float x, float y, float z)
{
	glRotatef(1.0f, x, y, z);
}

void OpenGL_Renderer::Render_Mesh(int id)
{
	Mesh& mesh = m_meshs[id];
	DBG_ASSERT(mesh.active == true);
	
#ifdef USE_VBO_FOR_MESH
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER_ARB, mesh.vbo_vertices_buffer);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER_ARB, mesh.vbo_texcoords_buffer);
	glTexCoordPointer(2, GL_FLOAT, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER_ARB, mesh.vbo_normals_buffer);
	glNormalPointer(GL_FLOAT, 0, NULL);

	glDrawArrays(GL_TRIANGLES, 0, mesh.vbo_vertices_count);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#else
	glCallList(mesh.list_id);
#endif
}

void OpenGL_Renderer::Destroy_Mesh(int id)
{
	Mesh& mesh = m_meshs[id];
	DBG_ASSERT(mesh.active == true);

#ifdef USE_VBO_FOR_MESH
	glDeleteBuffers(1, &mesh.vbo_vertices_buffer);
	glDeleteBuffers(1, &mesh.vbo_texcoords_buffer);
	glDeleteBuffers(1, &mesh.vbo_normals_buffer);

	SAFE_DELETE_ARRAY(mesh.vbo_vertices);
	SAFE_DELETE_ARRAY(mesh.vbo_texcoords);
	SAFE_DELETE_ARRAY(mesh.vbo_normals);
#else
	glDeleteLists(mesh.list_id, 1);
#endif
}

int OpenGL_Renderer::Start_Mesh(int vertices, int triangles)
{
	int mesh_id = Get_Free_Mesh_Index();
	if (mesh_id < 0)
	{
		return mesh_id;
	}

	Mesh& mesh = m_meshs[mesh_id];
	mesh.active				= true;
	mesh.vertices			= new Vertex[vertices];
	mesh.triangles			= new Triangle[triangles];
	mesh.vertex_counter		= 0;
	mesh.triangle_counter	= 0;
	mesh.vertex_count		= vertices;
	mesh.triangle_count		= triangles;

	#ifdef USE_VBO_FOR_MESH
		glGenBuffers(1, &mesh.vbo_vertices_buffer);
		glGenBuffers(1, &mesh.vbo_texcoords_buffer);
		glGenBuffers(1, &mesh.vbo_normals_buffer);
	#else
		mesh.list_id = glGenLists(1);
	#endif

	return mesh_id;
}

void OpenGL_Renderer::End_Mesh(int id)
{		
	DBG_ASSERT(id >= 0);

	Mesh& mesh = m_meshs[id];
	DBG_ASSERT(mesh.active == true);
	DBG_ASSERT(mesh.vertex_counter == mesh.vertex_counter);
	DBG_ASSERT(mesh.triangle_counter == mesh.triangle_count);
	
#ifdef USE_VBO_FOR_MESH
	mesh.vbo_vertices_count = mesh.triangle_count * 3;
	mesh.vbo_normals		= new float[mesh.vbo_vertices_count * 3];
	mesh.vbo_vertices		= new float[mesh.vbo_vertices_count * 3];
	mesh.vbo_texcoords		= new float[mesh.vbo_vertices_count * 2];

	int tex_coord_offset = 0;
	int normal_offset = 0;
	int vertex_offset = 0;
	for (int i = 0; i < mesh.triangle_count; i++)
	{
		Triangle& tri = mesh.triangles[i];
		Vertex&	v1  = mesh.vertices[tri.vertex_1];
		Vertex&	v2  = mesh.vertices[tri.vertex_2];
		Vertex&	v3  = mesh.vertices[tri.vertex_3];
		
		mesh.vbo_texcoords[tex_coord_offset++] = v1.u;
		mesh.vbo_texcoords[tex_coord_offset++] = v1.v;		
		mesh.vbo_normals[normal_offset++] = v1.normal.X;
		mesh.vbo_normals[normal_offset++] = v1.normal.Y;
		mesh.vbo_normals[normal_offset++] = v1.normal.Z;
		mesh.vbo_vertices[vertex_offset++] = v1.position.X;
		mesh.vbo_vertices[vertex_offset++] = v1.position.Y;
		mesh.vbo_vertices[vertex_offset++] = v1.position.Z;
		
		mesh.vbo_texcoords[tex_coord_offset++] = v2.u;
		mesh.vbo_texcoords[tex_coord_offset++] = v2.v;		
		mesh.vbo_normals[normal_offset++] = v2.normal.X;
		mesh.vbo_normals[normal_offset++] = v2.normal.Y;
		mesh.vbo_normals[normal_offset++] = v2.normal.Z;
		mesh.vbo_vertices[vertex_offset++] = v2.position.X;
		mesh.vbo_vertices[vertex_offset++] = v2.position.Y;
		mesh.vbo_vertices[vertex_offset++] = v2.position.Z;
		
		mesh.vbo_texcoords[tex_coord_offset++] = v3.u;
		mesh.vbo_texcoords[tex_coord_offset++] = v3.v;		
		mesh.vbo_normals[normal_offset++] = v3.normal.X;
		mesh.vbo_normals[normal_offset++] = v3.normal.Y;
		mesh.vbo_normals[normal_offset++] = v3.normal.Z;
		mesh.vbo_vertices[vertex_offset++] = v3.position.X;
		mesh.vbo_vertices[vertex_offset++] = v3.position.Y;
		mesh.vbo_vertices[vertex_offset++] = v3.position.Z;
	}

	glBindBuffer(GL_ARRAY_BUFFER_ARB, mesh.vbo_vertices_buffer);
	glBufferData(GL_ARRAY_BUFFER_ARB, mesh.vbo_vertices_count * sizeof(float) * 3, mesh.vbo_vertices, GL_STATIC_DRAW_ARB);
	
	glBindBuffer(GL_ARRAY_BUFFER_ARB, mesh.vbo_normals_buffer);
	glBufferData(GL_ARRAY_BUFFER_ARB, mesh.vbo_vertices_count * sizeof(float) * 3, mesh.vbo_normals, GL_STATIC_DRAW_ARB);
	
	glBindBuffer(GL_ARRAY_BUFFER_ARB, mesh.vbo_texcoords_buffer);
	glBufferData(GL_ARRAY_BUFFER_ARB, mesh.vbo_vertices_count * sizeof(float) * 2, mesh.vbo_texcoords, GL_STATIC_DRAW_ARB);
#else
	glNewList(mesh.list_id, GL_COMPILE);
	glBegin(GL_TRIANGLES);

	// Compile triangles.
	for (int i = 0; i < mesh.triangle_count; i++)
	{
		Triangle& tri = mesh.triangles[i];
		Vertex&	v1  = mesh.vertices[tri.vertex_1];
		Vertex&	v2  = mesh.vertices[tri.vertex_2];
		Vertex&	v3  = mesh.vertices[tri.vertex_3];

		glTexCoord2f(v1.u, v1.v);
		glNormal3f(v1.normal.X, v1.normal.Y, v1.normal.Z);
		glVertex3f(v1.position.X, v1.position.Y, v1.position.Z);
		//glColor4f(v1.r, v1.g, v1.b, v1.a);
		
		glTexCoord2f(v2.u, v2.v);
		glNormal3f(v2.normal.X, v2.normal.Y, v2.normal.Z);
		glVertex3f(v2.position.X, v2.position.Y, v2.position.Z);
		//glColor4f(v2.r, v2.g, v2.b, v2.a);
		
		glTexCoord2f(v3.u, v3.v);
		glNormal3f(v3.normal.X, v3.normal.Y, v3.normal.Z);
		glVertex3f(v3.position.X, v3.position.Y, v3.position.Z);
		//glColor4f(v3.r, v3.g, v3.b, v3.a);
	}

	glEnd();
	glEndList(); 
#endif

	// Delete triangle/vertex arrays.
	SAFE_DELETE_ARRAY(mesh.triangles);
	SAFE_DELETE_ARRAY(mesh.vertices);
}

int OpenGL_Renderer::Add_Mesh_Vertex(int id, Vector3 position, Vector3 normal, float r, float g, float b, float a, float u, float v)
{
	DBG_ASSERT(id >= 0);

	Mesh& mesh = m_meshs[id];
	DBG_ASSERT(mesh.active == true);
	DBG_ASSERT(mesh.vertex_counter < mesh.vertex_count);

	Vertex tri;
	tri.position	= position;
	tri.normal		= normal;
	tri.r			= r;
	tri.g			= g;
	tri.b			= b;
	tri.a			= a;
	tri.u			= u;
	tri.v			= v;

	mesh.vertices[mesh.vertex_counter] = tri;
	return mesh.vertex_counter++;
}

int OpenGL_Renderer::Add_Mesh_Triangle(int id, int vertex1, int vertex2, int vertex3)
{
	DBG_ASSERT(id >= 0);

	Mesh& mesh = m_meshs[id];
	DBG_ASSERT(mesh.active == true);
	DBG_ASSERT(mesh.triangle_counter < mesh.triangle_count);

	Triangle tri;
	tri.vertex_1 = vertex1;
	tri.vertex_2 = vertex2;
	tri.vertex_3 = vertex3;

	mesh.triangles[mesh.triangle_counter] = tri;
	return mesh.triangle_counter++;
}

void OpenGL_Renderer::Draw_Wireframe_Sphere(float r)
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
    
		glBegin(GL_LINE_STRIP);
		for (int j = 0; j <= longs; j++) 
		{
			double lng = 2 * PI * (double) (j - 1) / longs;
			double x = cos(lng);
			double y = sin(lng);
    
			glNormal3f((x * zr0) * r, (y * zr0) * r, z0 * r);
			glVertex3f((x * zr0) * r, (y * zr0) * r, z0 * r);
			glNormal3f((x * zr1) * r, (y * zr1) * r, z1 * r);
			glVertex3f((x * zr1) * r, (y * zr1) * r, z1 * r);
		}
		glEnd();
	}
}

void OpenGL_Renderer::Draw_Wireframe_Cube(float w, float h, float d)
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

void OpenGL_Renderer::Draw_Line(float x1, float y1, float z1, float x2, float y2, float z2)
{
	glLineWidth(2.5f); 
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(x1, y1, z1);
	glVertex3f(x2, y2, z2);
	glEnd();
}