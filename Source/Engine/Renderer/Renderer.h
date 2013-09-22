// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_
#define _ENGINE_RENDERER_

#include "Engine\Engine\FrameTime.h"
#include "Generic\Patterns\Singleton.h"
#include "Generic\Types\Vector3.h"
#include "Generic\Types\Matrix4.h"
#include "Generic\Types\AABB.h"
#include "Generic\Types\Frustum.h"
#include "Generic\Types\Color.h"
#include "Generic\Types\Rectangle.h"

#include "Engine\Renderer\Textures\Texture.h"
#include "Engine\Renderer\Textures\RenderTarget.h"
#include "Engine\Renderer\Shaders\Shader.h"
#include "Engine\Renderer\Material.h"

#include <vector>

class Display;
class Camera;
class ShaderProgram;

struct RendererOption
{
	enum Type
	{
#define STATE_SETTING(setting_name, setting_enum) \
		setting_enum, 
#include "Engine\Renderer\Renderer_RenderOptions.inc"
#undef STATE_SETTING
	};
};

class Renderer : public Singleton<Renderer>
{
public:
	static Renderer* Create();

	// Base functions.	
	virtual void			Flip						(const FrameTime& time) = 0;

	// Display related settings.
	virtual bool			Set_Display					(Display* display) = 0;

	// Rendering! What we are all here for.
	virtual void			Bind_Render_Target			(RenderTarget* texture) = 0;
	virtual void			Bind_Material				(Material* material) = 0;
	virtual void			Bind_Texture				(Texture* texture, int slot) = 0;
	virtual void			Bind_Shader_Program			(ShaderProgram* program) = 0;

	virtual Texture*		Create_Texture				(char* data, int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags) = 0;
	virtual Texture*		Create_Texture				(int width, int height, int pitch, TextureFormat::Type format, TextureFlags::Type flags) = 0;
	virtual Shader*			Create_Shader				(char* source, ShaderType::Type type) = 0;
	virtual ShaderProgram*  Create_Shader_Program		(std::vector<Shader*>& shaders) = 0;

	virtual RenderTarget*	Create_Render_Target		() = 0;

	virtual Material*		Get_Material				() = 0;

	virtual void			Set_Output_Buffers			(std::vector<OutputBufferType::Type>& outputs) = 0;

	virtual void					Set_Clear_Color				(Color color) = 0;
	virtual Color					Get_Clear_Color				() = 0;
	virtual void					Set_Clear_Depth				(float depth) = 0;
	virtual float					Get_Clear_Depth				() = 0;
	virtual void					Set_Cull_Face				(RendererOption::Type option) = 0;
	virtual RendererOption::Type	Get_Cull_Face				() = 0;
	virtual void					Set_Depth_Function			(RendererOption::Type option) = 0;
	virtual RendererOption::Type	Get_Depth_Function			() = 0;
	virtual void					Set_Depth_Test				(bool depth) = 0;
	virtual bool					Get_Depth_Test				() = 0;
	virtual void					Set_Blend_Function			(RendererOption::Type option) = 0;
	virtual RendererOption::Type	Get_Blend_Function			() = 0;
	virtual void					Set_Blend					(bool blend) = 0;
	virtual bool					Get_Blend					() = 0;

	virtual void					Set_Viewport				(Rect viewport) = 0;
	virtual Rect					Set_Viewport				() = 0;

	virtual void					Clear_Buffer				() = 0;

	virtual void			Set_World_Matrix			(Matrix4 matrix) = 0;
	virtual Matrix4			Get_World_Matrix			() = 0;
	virtual void			Set_View_Matrix				(Matrix4 matrix) = 0;
	virtual Matrix4			Get_View_Matrix				() = 0;
	virtual void			Set_Projection_Matrix		(Matrix4 matrix) = 0;
	virtual Matrix4			Get_Projection_Matrix		() = 0;

	virtual void			Render_Mesh					(int id) = 0;
	virtual void			Destroy_Mesh				(int id) = 0;
	virtual int				Start_Mesh					(int vertices, int triangles) = 0;
	virtual void			End_Mesh					(int id) = 0;
	virtual int				Add_Mesh_Vertex				(int id, Vector3 position, Vector3 normal, float r, float g, float b, float a, float u, float v) = 0;
	virtual int				Add_Mesh_Triangle			(int id, int vertex1, int vertex2, int vertex3) = 0;

	// Immediate rendering (mainly used for debugging).
	virtual void			Draw_Wireframe_Cube			(float w, float h, float d) = 0;
	virtual void			Draw_Wireframe_Sphere		(float r) = 0;
	virtual void			Draw_Line					(float x1, float y1, float z1, float x2, float y2, float z2) = 0;

};

#endif

