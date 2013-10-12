// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_OPENGL_OPENGL_RENDERER_
#define _ENGINE_RENDERER_OPENGL_OPENGL_RENDERER_

#include "Generic\Patterns\Singleton.h"
#include "Generic\Types\Matrix4.h"
#include "Engine\Renderer\Renderer.h"

#include "Engine\Renderer\OpenGL\OpenGL_Shader.h"
#include "Engine\Renderer\OpenGL\OpenGL_ShaderFactory.h"

#ifdef PLATFORM_WIN32
#include <Windows.h>
#include "Engine\Display\Win32\Win32_Display.h"
#endif

#include <vector>

class Display;
class OpenGL_Texture;
class OpenGL_ShaderProgram;
class OpenGL_RenderTarget;

// If this is not defined then display lists will be used.
#define USE_VBO_FOR_MESH

// Maximum number of meshs that can exist at one time. Theoretical max of this is load_distance_w*load_distance_h*load_Distance_d
#define MAX_MESHS				1024 * 256

// Maximum number of textures to bind at once.
#define MAX_BINDED_TEXTURES		8

// OpenGL extensions.
extern PFNGLGENBUFFERSARBPROC				glGenBuffers;
extern PFNGLBINDBUFFERARBPROC				glBindBuffer;
extern PFNGLBUFFERDATAARBPROC				glBufferData;
extern PFNGLDELETEBUFFERSARBPROC			glDeleteBuffers;

extern PFNGLCREATESHADERPROC				glCreateShader;
extern PFNGLDELETESHADERPROC				glDeleteShader;
extern PFNGLATTACHSHADERPROC				glAttachShader;
extern PFNGLSHADERSOURCEPROC				glShaderSource;
extern PFNGLCOMPILESHADERPROC				glCompileShader;
extern PFNGLCREATEPROGRAMPROC				glCreateProgram;
extern PFNGLLINKPROGRAMPROC					glLinkProgram;
extern PFNGLDELETEPROGRAMPROC				glDeleteProgram;
extern PFNGLGETPROGRAMIVPROC				glGetProgramiv;
extern PFNGLGETSHADERIVPROC					glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC			glGetShaderInfoLog;
extern PFNGLGETPROGRAMINFOLOGPROC			glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC					glUseProgram;

extern PFNGLACTIVETEXTUREPROC				glActiveTexture;
extern PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation;
extern PFNGLUNIFORM1IPROC					glUniform1i;
extern PFNGLUNIFORM1FPROC					glUniform1f;
extern PFNGLUNIFORM3FPROC					glUniform3f;
extern PFNGLUNIFORM4FPROC					glUniform4f;
extern PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv;

extern PFNGLGENFRAMEBUFFERSPROC				glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC				glBindFramebuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC		glFramebufferTexture2D;

extern PFNGLCHECKFRAMEBUFFERSTATUSPROC		glCheckFramebufferStatus;		
extern PFNGLDRAWBUFFERSPROC					glDrawBuffers;		
	
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

	Mesh					m_meshs[MAX_MESHS];
	Display*				m_display;
	bool					m_initialized;
	
	// Maticies
	Matrix4					m_world_matrix;
	Matrix4					m_view_matrix;
	Matrix4					m_projection_matrix;

	const OpenGL_Texture*	m_binded_textures[MAX_BINDED_TEXTURES];
	OpenGL_RenderTarget*	m_binded_render_target;
	OpenGL_ShaderProgram*	m_binded_shader_program;
	Material*				m_binded_material;

	Color					m_clear_color;
	float					m_clear_depth;
	RendererOption::Type	m_cull_face;
	RendererOption::Type	m_depth_function;
	bool					m_depth_test;
	bool					m_alpha_test;
	Rect					m_viewport;
	RendererOption::Type	m_blend_function;
	bool					m_blend;

	// Initializes the opengl shader factory!
	OpenGL_ShaderFactory	m_shader_factory;

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
	void Flip(const FrameTime& time);

	// Display related settings.
	bool Set_Display(Display* display);
	
	// Rendering.		
	void Bind_Texture(const Texture* texture, int index);
	void Bind_Material(Material* material);
	void Bind_Shader_Program(ShaderProgram* texture);
	void Bind_Render_Target(RenderTarget* texture);

	Texture*		Create_Texture			(char* data, int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags);
	Texture*		Create_Texture			(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags);
	Shader*			Create_Shader			(char* source, ShaderType::Type type);
	ShaderProgram*  Create_Shader_Program	(std::vector<Shader*>& shaders);
	RenderTarget*	Create_Render_Target	();
	
	Material*		Get_Material			();
	
	void			Set_Output_Buffers		(std::vector<OutputBufferType::Type>& outputs);
			
	void					Set_Clear_Color				(Color color);
	Color					Get_Clear_Color				();
	void					Set_Clear_Depth				(float depth);
	float					Get_Clear_Depth				();
	void					Set_Cull_Face				(RendererOption::Type option);
	RendererOption::Type	Get_Cull_Face				();
	void					Set_Depth_Function			(RendererOption::Type option);
	RendererOption::Type	Get_Depth_Function			();
	void					Set_Depth_Test				(bool depth);
	bool					Get_Depth_Test				();
	void					Set_Alpha_Test				(bool depth);
	bool					Get_Alpha_Test				();
	void					Set_Blend_Function			(RendererOption::Type option);
	RendererOption::Type	Get_Blend_Function			();
	void					Set_Blend					(bool blend);
	bool					Get_Blend					();

	void					Set_Viewport				(Rect viewport);
	Rect					Set_Viewport				();

	void					Clear_Buffer				();

	void		Set_World_Matrix			(Matrix4 matrix);
	Matrix4		Get_World_Matrix			();
	void		Set_View_Matrix				(Matrix4 matrix);
	Matrix4		Get_View_Matrix				();
	void		Set_Projection_Matrix		(Matrix4 matrix);
	Matrix4		Get_Projection_Matrix		();

	void Render_Mesh(int id);
	void Destroy_Mesh(int id);
	int  Start_Mesh(int vertices, int triangles);
	void End_Mesh(int id);
	int  Add_Mesh_Vertex(int id, Vector3 position, Vector3 normal, float r, float g, float b, float a, float u, float v);
	int  Add_Mesh_Triangle(int id, int vertex1, int vertex2, int vertex3);

	// Immediate rendering.	
	void Draw_Line(float x1, float y1, float z1, float x2, float y2, float z2, float size = 1.0f);
	void Draw_Quad(Rect bounds, Rect uv);

};

#endif

