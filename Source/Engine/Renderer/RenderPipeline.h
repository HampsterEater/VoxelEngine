// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDER_PIPELINE_
#define _ENGINE_RENDER_PIPELINE_

#include "Engine\Engine\FrameTime.h"
#include "Generic\Patterns\Singleton.h"
#include "Generic\Types\Vector3.h"
#include "Generic\Types\Matrix4.h"
#include "Generic\Types\AABB.h"
#include "Generic\Types\Frustum.h"
#include "Generic\Types\Color.h"

#include "Engine\Renderer\Renderer.h"
#include "Engine\Renderer\Textures\TextureHandle.h"
#include "Engine\Renderer\Textures\Texture.h"
#include "Engine\Renderer\Shaders\Shader.h"
#include "Engine\Renderer\Textures\RenderTarget.h"
#include "Engine\Renderer\Material.h"

#include "Generic\ThirdParty\RapidXML\rapidxml.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_iterators.hpp"

#include "Engine\Resources\Reloadable.h"

#include <vector>

class Display;
class Camera;
class RenderTarget;
class ShaderProgram;
class Light;
class Drawable;
class Texture;

struct RenderPipeline_StateSettingType
{
	enum Type
	{
#define STATE_SETTING(setting_name, setting_enum, setting_type) \
		setting_enum, 
#include "Engine\Renderer\RenderPipeline_StateSettingType.inc"
#undef STATE_SETTING
	};
};

struct RenderPipeline_StateSettingValueType
{
	enum Type
	{
		// Standard types.
		Color,
		Bool,
		Float,
		String,
		Int,
		Enum
	};
};

struct RenderPipeline_StateSetting
{
	RenderPipeline_StateSettingType::Type	     Type;
	RenderPipeline_StateSettingValueType::Type	 ValueType;

	std::string				StringValue;
	Color					ColorValue;
	float					FloatValue;
	int						IntValue;
	bool					BoolValue;
	RendererOption::Type	EnumValue;
};

struct RenderPipeline_State
{
	RenderPipeline_State();
	~RenderPipeline_State();

	std::vector<RenderPipeline_StateSetting*>	Settings;
};

struct RenderPipeline_MeshPrimitive
{
	enum Type
	{
		Scene,
		Quads
	};
};

struct RenderPipeline_Mesh
{
	RenderPipeline_Mesh();
	~RenderPipeline_Mesh();

	std::string							Name;
	int									Mesh_ID;
};

struct RenderPipeline_Texture
{
	RenderPipeline_Texture();
	~RenderPipeline_Texture();

	std::string							Name;
	Texture*							RawTexture;
	TextureHandle*						Texture;
};

struct RenderPipeline_Target
{
	RenderPipeline_Target();
	~RenderPipeline_Target();

	RenderTarget*							Target;
	std::string								Name;
	std::vector<RenderPipeline_Texture*>	Attached_Textures;
};

struct RenderPipeline_ShaderUniformType
{
	enum Type
	{
		Texture,
	
#define SHADER_UNIFORM(uniform_type, uniform_name, uniform_constant) \
		uniform_constant, 
#include "Engine\Renderer\RenderPipeline_ShaderUniformType.inc"
#undef SHADER_UNIFORM

	};
};

struct RenderPipeline_ShaderUniform
{
	RenderPipeline_ShaderUniform();

	std::string								Name;
	RenderPipeline_ShaderUniformType::Type	Type;
	std::string								Value;
	RenderPipeline_Texture*					Texture;
};

struct RenderPipeline_Shader
{
	RenderPipeline_Shader();
	~RenderPipeline_Shader();

	std::string									Name;
	Shader*										Vertex_Shader;
	Shader*										Fragment_Shader;
	ShaderProgram*								Shader_Program;
	std::vector<RenderPipeline_ShaderUniform*>	Uniforms;
};

struct RenderPipeline_PassType
{
	enum Type
	{
		Scene,
		Mesh,
		Container
	};
};

struct RenderPipeline_PassForEachType
{
	enum Type
	{
		None,
		Light,
		Shadow_Casting_Light
	};
};

struct RenderPipeline_PassOutputType
{
	enum Type
	{
		Texture,
		
		// Special "constant" outputs.
		BackBuffer
	};
};

struct RenderPipeline_PassOutput
{
	RenderPipeline_PassOutput();

	RenderPipeline_PassOutputType::Type Type;
	RenderPipeline_Texture*				Texture;
};

struct RenderPipeline_Pass
{
	RenderPipeline_Pass();
	~RenderPipeline_Pass();

	std::string									Name;
	RenderPipeline_PassType::Type				Type;
	bool										Enabled;
	RenderPipeline_Shader*						Shader;
	RenderPipeline_Target*						Target;
	RenderPipeline_Mesh*						Mesh;
	RenderPipeline_State*						State;
	std::vector<RenderPipeline_PassOutput*>		Outputs;
	
	RenderPipeline_PassForEachType::Type		Foreach;
	std::vector<RenderPipeline_Pass*>			SubPasses;
};

struct RenderPipeline_SlotSortType
{
	enum Type
	{
		None,
		Front_To_Back
	};
};

struct RenderPipeline_Slot
{
	RenderPipeline_Slot();
	~RenderPipeline_Slot();

	std::string									Name;
	int											NameHash;
	RenderPipeline_Shader*						Shader;
	RenderPipeline_SlotSortType::Type			SortType;
	std::vector<RenderPipeline_Pass*>			Passes;

	static bool Sort_Front_To_Back(Drawable* a, Drawable* b); 
};

class RenderPipeline : public Singleton<RenderPipeline>, public Reloadable
{
private:
	RenderPipeline_State*					m_default_state;
	std::vector<RenderPipeline_Mesh*>		m_meshes;
	std::vector<RenderPipeline_Texture*>	m_textures;
	std::vector<RenderPipeline_Target*>		m_targets;
	std::vector<RenderPipeline_Shader*>		m_shaders;
	std::vector<RenderPipeline_Pass*>		m_passes;
	std::vector<RenderPipeline_Slot*>		m_slots;

	Renderer*								m_renderer;

	RenderPipeline_Shader*					m_binded_shader_program;

	Camera*									m_active_camera;
	Light*									m_active_light;

	std::string								m_config_path;

	RenderPipeline_Pass*					m_current_pass;

protected:
	std::string Get_Attribute_Value(rapidxml::xml_node<>* node, const char* name, const char* def);
	std::string Get_Node_Value	   (rapidxml::xml_node<>* node, const char* name, const char* def);

	// Drawing functions.
	void Draw_Pass(const FrameTime& time, RenderPipeline_Pass* pass);
	void Draw_Scene(const FrameTime& time);
	void Draw_Scene_With_Matrices(const FrameTime& time, Matrix4& projection_matrix, Matrix4& view_matrix, Matrix4& world_matrix);

	// Loading functions.
	void Load_Defaults	(rapidxml::xml_node<>* node);
	void Load_Meshes	(rapidxml::xml_node<>* node);
	void Load_Textures	(rapidxml::xml_node<>* node);
	void Load_Targets	(rapidxml::xml_node<>* node);
	void Load_Shaders	(rapidxml::xml_node<>* node);
	void Load_Passes	(rapidxml::xml_node<>* node);
	void Load_Slots		(rapidxml::xml_node<>* node);
	
	RenderPipeline_State*	Load_State	(rapidxml::xml_node<>* node);
	RenderPipeline_Mesh*	Load_Mesh	(rapidxml::xml_node<>* node);
	RenderPipeline_Texture*	Load_Texture(rapidxml::xml_node<>* node);
	RenderPipeline_Target*	Load_Target	(rapidxml::xml_node<>* node);
	RenderPipeline_Shader*	Load_Shader	(rapidxml::xml_node<>* node);
	RenderPipeline_Pass*	Load_Pass	(rapidxml::xml_node<>* node);
	RenderPipeline_Slot*	Load_Slot	(rapidxml::xml_node<>* node);

public:

	// Construction functions.
	RenderPipeline(Renderer* renderer);
	~RenderPipeline();

	// Configuration functions.
	bool Load_Config(const char* path);
	void Reset();
	void Update_Shader_Uniforms();

	// Base functions.	
	void Draw(const FrameTime& time);
	void Draw_Game(const FrameTime& time);
	void Reload();

	// Get/Set method.
	Camera*					Get_Active_Camera		();
	Light*					Get_Active_Light		();

	void					Set_Active_Camera		(Camera* camera);

	RenderPipeline_Texture* Get_Texture_From_Name	(const char* name);
	RenderPipeline_Mesh*    Get_Mesh_From_Name		(const char* name);
	RenderPipeline_Target*  Get_Target_From_Name	(const char* name);
	RenderPipeline_Shader*  Get_Shader_From_Name	(const char* name);
	RenderPipeline_Pass*	Get_Pass_From_Name		(const char* name);
	RenderPipeline_Slot*	Get_Slot_From_Name		(const char* name);

	void Apply_Outputs(const FrameTime& time, RenderPipeline_Pass* pass);
	void Apply_State(const FrameTime& time, RenderPipeline_State* state);
	void Apply_Shader(const FrameTime& time, RenderPipeline_Shader* shader);
};

#endif
