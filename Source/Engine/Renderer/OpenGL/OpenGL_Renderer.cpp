// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\OpenGL\OpenGL_Renderer.h"
#include "Engine\Display\Display.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Scene\Actor.h"
#include "Engine\Scene\Camera.h"
#include "Engine\Scene\Light.h"
#include "Engine\Renderer\Drawable.h"
#include "Engine\Engine\GameEngine.h"

#include "Generic\Math\Math.h"
#include "Generic\Types\Matrix4.h"
#include "Generic\Types\Rectangle.h"

#include "Engine\Renderer\Textures\TextureFactory.h"

#include "Engine\Renderer\OpenGL\OpenGL_Texture.h"
#include "Engine\Renderer\OpenGL\OpenGL_Shader.h"
#include "Engine\Renderer\OpenGL\OpenGL_ShaderProgram.h"
#include "Engine\Renderer\OpenGL\OpenGL_RenderTarget.h"

#ifdef PLATFORM_WIN32
#include "Engine\Display\Win32\Win32_Display.h"
#endif

#include "Engine\IO\StreamFactory.h"
#include "Engine\IO\Stream.h"

#include <vector>

// Does some debug checking when loading gl extensions.
#define LOAD_GL_EXTENSION(func, type, name) \
	{ \
		func = (type)wglGetProcAddress(#name); \
		DBG_ASSERT(func != NULL); \
	}

// Extension methods to opengl that we need.
PFNGLGENBUFFERSARBPROC				glGenBuffers				= NULL;                  
PFNGLBINDBUFFERARBPROC				glBindBuffer				= NULL;                 
PFNGLBUFFERDATAARBPROC				glBufferData				= NULL;                
PFNGLDELETEBUFFERSARBPROC			glDeleteBuffers				= NULL;  

PFNGLCREATESHADERPROC				glCreateShader				= NULL;
PFNGLDELETESHADERPROC				glDeleteShader				= NULL;
PFNGLATTACHSHADERPROC				glAttachShader				= NULL;
PFNGLSHADERSOURCEPROC				glShaderSource				= NULL;
PFNGLCOMPILESHADERPROC				glCompileShader				= NULL;
PFNGLCREATEPROGRAMPROC				glCreateProgram				= NULL;
PFNGLLINKPROGRAMPROC				glLinkProgram				= NULL;
PFNGLDELETEPROGRAMPROC				glDeleteProgram				= NULL;
PFNGLGETPROGRAMIVPROC				glGetProgramiv				= NULL;
PFNGLGETSHADERIVPROC				glGetShaderiv				= NULL;
PFNGLGETSHADERINFOLOGPROC			glGetShaderInfoLog			= NULL;
PFNGLGETPROGRAMINFOLOGPROC			glGetProgramInfoLog			= NULL;
PFNGLUSEPROGRAMPROC					glUseProgram				= NULL;

PFNGLACTIVETEXTUREPROC				glActiveTexture				= NULL;
PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation		= NULL;
PFNGLUNIFORM1IPROC					glUniform1i					= NULL;
PFNGLUNIFORM1FPROC					glUniform1f					= NULL;
PFNGLUNIFORM3FPROC					glUniform3f					= NULL;
PFNGLUNIFORM4FPROC					glUniform4f					= NULL;
PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv 			= NULL;

PFNGLGENFRAMEBUFFERSPROC			glGenFramebuffers			= NULL;
PFNGLBINDFRAMEBUFFERPROC			glBindFramebuffer			= NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC		glFramebufferTexture2D		= NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC		glCheckFramebufferStatus	= NULL;
	
PFNGLDRAWBUFFERSPROC				glDrawBuffers				= NULL;		

#ifdef PLATFORM_WIN32
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC) (int interval);              
PFNWGLSWAPINTERVALEXTPROC			wglSwapInterval				= NULL;  
#endif

OpenGL_Renderer::OpenGL_Renderer()
	: m_initialized(false)
	, m_display(NULL)
	, m_binded_shader_program(NULL)
	, m_binded_render_target(NULL)
	, m_binded_material(NULL)
	, m_viewport(0, 0, 0, 0)
{
	memset(m_binded_textures, 0, sizeof(OpenGL_Texture*) * MAX_BINDED_TEXTURES);
}

OpenGL_Renderer::~OpenGL_Renderer()
{
	// TODO: Destroy all shaders etc.

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

	LOAD_GL_EXTENSION(glGenBuffers,				PFNGLGENBUFFERSARBPROC,				glGenBuffersARB);
	LOAD_GL_EXTENSION(glBindBuffer,				PFNGLBINDBUFFERARBPROC,				glBindBufferARB);
	LOAD_GL_EXTENSION(glBufferData,				PFNGLBUFFERDATAARBPROC,				glBufferDataARB);
	LOAD_GL_EXTENSION(glDeleteBuffers,			PFNGLDELETEBUFFERSARBPROC,			glDeleteBuffersARB);
	LOAD_GL_EXTENSION(wglSwapInterval,			PFNWGLSWAPINTERVALEXTPROC,			wglSwapIntervalEXT);
	LOAD_GL_EXTENSION(glCreateShader,			PFNGLCREATESHADERPROC,				glCreateShader);
	LOAD_GL_EXTENSION(glDeleteShader,			PFNGLDELETESHADERPROC,				glDeleteShader);
	LOAD_GL_EXTENSION(glAttachShader,			PFNGLATTACHSHADERPROC,				glAttachShader);
	LOAD_GL_EXTENSION(glShaderSource,			PFNGLSHADERSOURCEPROC,				glShaderSource);
	LOAD_GL_EXTENSION(glCompileShader,			PFNGLCOMPILESHADERPROC,				glCompileShader);
	LOAD_GL_EXTENSION(glCreateProgram,			PFNGLCREATEPROGRAMPROC,				glCreateProgram);
	LOAD_GL_EXTENSION(glLinkProgram,			PFNGLLINKPROGRAMPROC,				glLinkProgram);
	LOAD_GL_EXTENSION(glDeleteProgram,			PFNGLDELETEPROGRAMPROC,				glDeleteProgram);
	LOAD_GL_EXTENSION(glGetProgramiv,			PFNGLGETPROGRAMIVPROC,				glGetProgramiv);
	LOAD_GL_EXTENSION(glGetShaderiv,			PFNGLGETSHADERIVPROC,				glGetShaderiv);
	LOAD_GL_EXTENSION(glGetShaderInfoLog,		PFNGLGETSHADERINFOLOGPROC,			glGetShaderInfoLog);
	LOAD_GL_EXTENSION(glGetProgramInfoLog,		PFNGLGETPROGRAMINFOLOGPROC,			glGetProgramInfoLog);
	LOAD_GL_EXTENSION(glUseProgram,				PFNGLUSEPROGRAMPROC,				glUseProgram);
	LOAD_GL_EXTENSION(glActiveTexture,			PFNGLACTIVETEXTUREPROC,				glActiveTexture);
	LOAD_GL_EXTENSION(glGetUniformLocation,		PFNGLGETUNIFORMLOCATIONPROC,		glGetUniformLocation);
	LOAD_GL_EXTENSION(glUniform1i,				PFNGLUNIFORM1IPROC,					glUniform1i);
	LOAD_GL_EXTENSION(glUniform1f,				PFNGLUNIFORM1FPROC,					glUniform1f);
	LOAD_GL_EXTENSION(glUniform3f,				PFNGLUNIFORM3FPROC,					glUniform3f);
	LOAD_GL_EXTENSION(glUniform4f,				PFNGLUNIFORM4FPROC,					glUniform4f);
	LOAD_GL_EXTENSION(glUniformMatrix4fv,		PFNGLUNIFORMMATRIX4FVPROC,			glUniformMatrix4fv);
	LOAD_GL_EXTENSION(glGenFramebuffers,		PFNGLGENFRAMEBUFFERSPROC,			glGenFramebuffers);
	LOAD_GL_EXTENSION(glBindFramebuffer,		PFNGLBINDFRAMEBUFFERPROC,			glBindFramebuffer);
	LOAD_GL_EXTENSION(glFramebufferTexture2D,	PFNGLFRAMEBUFFERTEXTURE2DPROC,		glFramebufferTexture2D);
	LOAD_GL_EXTENSION(glCheckFramebufferStatus, PFNGLCHECKFRAMEBUFFERSTATUSPROC,	glCheckFramebufferStatus);
	LOAD_GL_EXTENSION(glDrawBuffers,			PFNGLDRAWBUFFERSPROC,				glDrawBuffers);

	// Turn off vsync.
	wglSwapInterval(0);

	for (int i = 0; i < MAX_MESHS; i++)
	{
		m_meshs[i].active = false;
	}

	// Clear screen.							
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);	
	glDisable(GL_LIGHTING);

	Set_Clear_Color(Color::Black);
	Set_Clear_Depth(1.0f);
	Set_Depth_Test(true);
	Set_Depth_Function(RendererOption::Lower_Or_Equal);
	Set_Cull_Face(RendererOption::Back);
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

void OpenGL_Renderer::Flip(const FrameTime& time)
{
	// Swap front/back buffers.
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

void OpenGL_Renderer::Bind_Shader_Program(ShaderProgram* shader)
{
	OpenGL_ShaderProgram* glShader = dynamic_cast<OpenGL_ShaderProgram*>(shader);

	if (m_binded_shader_program != glShader)
	{
		if (glShader != NULL)
		{
			glUseProgram(glShader->Get_ID());
		}
		else
		{
			glUseProgram(0);
		}

		m_binded_shader_program = glShader;
	}	
}

Shader* OpenGL_Renderer::Create_Shader(char* source, ShaderType::Type type)
{
	GLuint shader = -1;

	switch (type)
	{
		case ShaderType::Vertex:	
			{
				shader = glCreateShader(GL_VERTEX_SHADER);		
				break;
			}
		case ShaderType::Fragment:	
			{
				shader = glCreateShader(GL_FRAGMENT_SHADER);	
				break;
			}
	}

	if (shader < 0)
	{
		return NULL;
	}

	glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
	
	GLint	result		= GL_FALSE;
	int		logLength	= 0;
	char*	log			= NULL;

    glGetShaderiv(shader, GL_COMPILE_STATUS,  &result);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 1)
	{
		log = new char[logLength + 1];
		memset(log, 0, logLength + 1);
		glGetShaderInfoLog(shader, logLength, NULL, log);

		DBG_LOG("=== Shader compile log ===\n%s\n", log);

		SAFE_DELETE(log);

		return NULL;
	}
	
 	if (result != GL_TRUE)
	{
		return NULL;
	}
	else
	{
		return new OpenGL_Shader(shader, source, type);
	}
}

ShaderProgram* OpenGL_Renderer::Create_Shader_Program(std::vector<Shader*>& shaders)
{
	GLuint program = glCreateProgram();

	for (std::vector<Shader*>::iterator iter = shaders.begin(); iter != shaders.end(); iter++)
	{
		OpenGL_Shader* shader = dynamic_cast<OpenGL_Shader*>(*iter);
		glAttachShader(program, shader->Get_ID());
	}

	glLinkProgram(program);
	
	GLint	result = GL_FALSE;
	int		logLength = 0;
	char*	log = NULL;

    glGetProgramiv(program, GL_LINK_STATUS,  &result);
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength > 1)
	{
		log = new char[logLength + 1];
		memset(log, 0, logLength + 1);
		glGetProgramInfoLog(program, logLength, NULL, log);

		DBG_LOG("=== Shader program link log ===\n%s", log);

		SAFE_DELETE(log);

		return NULL;
	}

	if (result != GL_TRUE)
	{
		return NULL;
	}
	else
	{
		return new OpenGL_ShaderProgram(program, shaders);
	}
}

void OpenGL_Renderer::Bind_Render_Target(RenderTarget* target)
{
	OpenGL_RenderTarget* glTarget = dynamic_cast<OpenGL_RenderTarget*>(target);

	if (m_binded_render_target != glTarget)
	{
		// Make sure commands on previous buffer are complete.
		glFlush();

		if (glTarget != NULL)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, glTarget->Get_ID());
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		m_binded_render_target = glTarget;
	}
}

RenderTarget* OpenGL_Renderer::Create_Render_Target()
{
	GLuint id = 0;
	glGenFramebuffers(1, &id);
	
	if (id >= 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, id);
		return new OpenGL_RenderTarget(id);
	}
	else
	{
		return NULL;
	}
}

void OpenGL_Renderer::Bind_Texture(const Texture* texture, int index)
{
	const OpenGL_Texture* glTexture = dynamic_cast<const OpenGL_Texture*>(texture);

	DBG_ASSERT(index >= 0 && index < MAX_BINDED_TEXTURES);

	if (m_binded_textures[index] != glTexture)
	{
		glActiveTexture(GL_TEXTURE0 + index);

		if (glTexture == NULL)
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, glTexture->Get_ID());
		}
		
		m_binded_textures[index] = glTexture;
	}
}

void OpenGL_Renderer::Bind_Material(Material* material)
{
	//if (m_binded_material != material)
	//{
		Bind_Texture(material->Get_Texture()->Get(), 0);

		m_binded_material = material;
	//}
}
	
Material* OpenGL_Renderer::Get_Material()
{
	return m_binded_material;
}
	
void OpenGL_Renderer::Set_Output_Buffers(std::vector<OutputBufferType::Type>& outputs)
{
	GLenum real_outputs[16];
	DBG_ASSERT(outputs.size() < 16);

	int index = 0;
	for (std::vector<OutputBufferType::Type>::iterator iter = outputs.begin(); iter != outputs.end(); iter++, index++)
	{
		OutputBufferType::Type type = *iter;
		switch (type)
		{
		case OutputBufferType::BackBuffer:				real_outputs[index] = GL_BACK;				break;
		case OutputBufferType::RenderTargetTexture0:	real_outputs[index] = GL_COLOR_ATTACHMENT0;	break;
		case OutputBufferType::RenderTargetTexture1:	real_outputs[index] = GL_COLOR_ATTACHMENT1;	break;
		case OutputBufferType::RenderTargetTexture2:	real_outputs[index] = GL_COLOR_ATTACHMENT2;	break;
		case OutputBufferType::RenderTargetTexture3:	real_outputs[index] = GL_COLOR_ATTACHMENT3;	break;
		case OutputBufferType::RenderTargetTexture4:	real_outputs[index] = GL_COLOR_ATTACHMENT4;	break;
		case OutputBufferType::RenderTargetTexture5:	real_outputs[index] = GL_COLOR_ATTACHMENT5;	break;
		case OutputBufferType::RenderTargetTexture6:	real_outputs[index] = GL_COLOR_ATTACHMENT6;	break;
		case OutputBufferType::RenderTargetTexture7:	real_outputs[index] = GL_COLOR_ATTACHMENT7;	break;
		case OutputBufferType::RenderTargetTexture8:	real_outputs[index] = GL_COLOR_ATTACHMENT8;	break;
		case OutputBufferType::RenderTargetTexture9:	real_outputs[index] = GL_COLOR_ATTACHMENT9;	break;
		default:										DBG_ASSERT(false);							break;
		}
	}

	glDrawBuffers(outputs.size(), real_outputs);
}

void OpenGL_Renderer::Set_Clear_Color(Color color)
{
	glClearColor(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f);
	m_clear_color = color;
}

Color OpenGL_Renderer::Get_Clear_Color()
{
	return m_clear_color;
}

void OpenGL_Renderer::Set_Clear_Depth(float depth)
{
	glClearDepth(depth);
	m_clear_depth = depth;
}

float OpenGL_Renderer::Get_Clear_Depth()
{
	return m_clear_depth;
}

void OpenGL_Renderer::Set_Cull_Face(RendererOption::Type option)
{
	glCullFace(option == RendererOption::Back ? GL_BACK : GL_FRONT);
	m_cull_face = option;
}

RendererOption::Type OpenGL_Renderer::Get_Cull_Face()
{
	return m_cull_face;
}

void OpenGL_Renderer::Set_Depth_Function(RendererOption::Type option)
{
	switch (option)
	{
		case RendererOption::Lower_Or_Equal:		glDepthFunc(GL_LEQUAL);		break;
		case RendererOption::Lower:					glDepthFunc(GL_LESS);		break;
		case RendererOption::Greater_Or_Equal:		glDepthFunc(GL_GEQUAL);		break;
		case RendererOption::Greater:				glDepthFunc(GL_GREATER);	break;
		case RendererOption::Equal:					glDepthFunc(GL_EQUAL);		break;
		case RendererOption::Not_Equal:				glDepthFunc(GL_NOTEQUAL);	break;
		case RendererOption::Never:					glDepthFunc(GL_NEVER);		break;
		case RendererOption::Always:				glDepthFunc(GL_ALWAYS);		break;
		default:									DBG_ASSERT(false);			break;
	}
	m_depth_function = option;
}

RendererOption::Type OpenGL_Renderer::Get_Depth_Function()
{
	return m_depth_function;
}

void OpenGL_Renderer::Set_Depth_Test(bool depth)
{
	if (depth)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	m_depth_test = depth;
}

bool OpenGL_Renderer::Get_Depth_Test()
{
	return m_depth_test;
}

void OpenGL_Renderer::Set_Alpha_Test(bool depth)
{
	if (depth)
	{
		glEnable(GL_ALPHA_TEST);
	}
	else
	{
		glDisable(GL_ALPHA_TEST);
	}

	m_alpha_test = depth;
}

bool OpenGL_Renderer::Get_Alpha_Test()
{
	return m_alpha_test;
}

void OpenGL_Renderer::Set_Blend_Function(RendererOption::Type option)
{
	switch (option)
	{
		case RendererOption::One_One:							glBlendFunc(GL_ONE, GL_ONE);						break;
		case RendererOption::Src_Alpha_One_Minus_Src_Alpha:		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	break;
		default:												DBG_ASSERT(false);									break;
	}
	m_blend_function = option;
}

RendererOption::Type OpenGL_Renderer::Get_Blend_Function()
{
	return m_blend_function;
}

void OpenGL_Renderer::Set_Blend(bool blend)
{
	if (blend)
	{
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	m_blend = blend;
}

bool OpenGL_Renderer::Get_Blend()
{
	return m_blend;
}

void OpenGL_Renderer::Set_Viewport(Rect viewport)
{
	glViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
	m_viewport = viewport;
}

Rect OpenGL_Renderer::Set_Viewport()
{
	return m_viewport;
}

void OpenGL_Renderer::Clear_Buffer()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Texture* OpenGL_Renderer::Create_Texture(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags)
{
	GLuint texture_id;	

	const OpenGL_Texture* previous_texture = m_binded_textures[0];

	// Generate and bind texture.
	glActiveTexture(0);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	
	// And some nice filtering and clamp the texture.
	if ((flags & TextureFlags::AllowRepeat) != 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	}
	if ((flags & TextureFlags::LinearFilter) != 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	}
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	
	// Upload data to GPU.
	switch (format)
	{
	case TextureFormat::R8G8B8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
			break;
		}
	case TextureFormat::R32FG32FB32FA32F:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);
			break;
		}
	case TextureFormat::R8G8B8A8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			break;
		}
	case TextureFormat::DepthFormat:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			break;
		}
	case TextureFormat::StencilFormat:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, width, height, 0, GL_STENCIL_INDEX, GL_FLOAT, 0);
			break;
		}
	case TextureFormat::Luminosity:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
			break;
		}

	default:
		{
			// Format not supported.
			DBG_ASSERT(false);
			break;
		}
	}

	// Reset binding to previous texture.
	if (previous_texture != NULL)
	{
		glBindTexture(GL_TEXTURE_2D, previous_texture->Get_ID());
	}

	return new OpenGL_Texture(texture_id, NULL, width, height, pitch, format);
}

Texture* OpenGL_Renderer::Create_Texture(char* data, int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags)
{
	GLuint texture_id;	
	
	const OpenGL_Texture* previous_texture = m_binded_textures[0];

	// Generate and bind texture.
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	
	// And some nice filtering and clamp the texture.
	glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);	
	if ((flags & TextureFlags::AllowRepeat) != 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	}
	if ((flags & TextureFlags::LinearFilter) != 0)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	}
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
	case TextureFormat::R32FG32FB32FA32F:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data);
			break;
		}
	case TextureFormat::R8G8B8A8:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			break;
		}
	case TextureFormat::DepthFormat:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data);
			break;
		}
	case TextureFormat::StencilFormat:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, width, height, 0, GL_STENCIL_INDEX, GL_FLOAT, data);
			break;
		}
	case TextureFormat::Luminosity:
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
			break;
		}

	default:
		{
			// Format not supported.
			DBG_ASSERT(false);
			break;
		}
	}

	// Reset binding to previous texture.
	if (previous_texture != NULL)
	{
		glBindTexture(GL_TEXTURE_2D, previous_texture->Get_ID());
	}

	return new OpenGL_Texture(texture_id, data, width, height, pitch, format);
}

void OpenGL_Renderer::Set_World_Matrix(Matrix4 matrix)
{
	m_world_matrix = matrix;
}

Matrix4 OpenGL_Renderer::Get_World_Matrix()
{
	return m_world_matrix;
}

void OpenGL_Renderer::Set_View_Matrix(Matrix4 matrix)
{
	m_view_matrix = matrix;
}

Matrix4 OpenGL_Renderer::Get_View_Matrix()
{
	return m_view_matrix;
}

void OpenGL_Renderer::Set_Projection_Matrix(Matrix4 matrix)
{
	m_projection_matrix = matrix;
}

Matrix4 OpenGL_Renderer::Get_Projection_Matrix()
{
	return m_projection_matrix;
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

void OpenGL_Renderer::Draw_Line(float x1, float y1, float z1, float x2, float y2, float z2, float size)
{
	glLineWidth(size); 
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(x1, y1, z1);
	glVertex3f(x2, y2, z2);
	glEnd();
}

void OpenGL_Renderer::Draw_Quad(Rect bounds, Rect uv)
{
	float left = bounds.X;
	float top = bounds.Y;
	float right = left + bounds.Width;
	float bottom = top + bounds.Height;

	float uv_left	= uv.X;
	float uv_right	= uv.X + uv.Width;
	float uv_top	= 1.0 - uv.Y;
	float uv_bottom = uv_top - uv.Height;

	glBegin(GL_QUADS);
		glTexCoord2f(uv_left, uv_top);
		glVertex3f(left, top, 0.0f);
				
		glTexCoord2f(uv_left, uv_bottom);
		glVertex3f(left, bottom, 0.0f);

		glTexCoord2f(uv_right, uv_bottom);
		glVertex3f(right, bottom, 0.0f);

		glTexCoord2f(uv_right, uv_top);
		glVertex3f(right, top, 0.0f);
	glEnd();
}

