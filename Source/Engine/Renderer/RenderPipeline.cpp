// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Renderer.h"
#include "Engine\Renderer\RenderPipeline.h"
#include "Engine\IO\StreamFactory.h"
#include "Engine\Display\Display.h"
#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Shaders\ShaderFactory.h"
#include "Engine\Renderer\Shaders\ShaderProgram.h"
#include "Engine\Scene\Light.h"
#include "Engine\Scene\Camera.h"
#include "Engine\Renderer\Drawable.h"
#include "Engine\Scene\Scene.h"
#include "Engine\Engine\GameEngine.h"

#include "Generic\Helper\StringHelper.h"

#include "Generic\ThirdParty\RapidXML\rapidxml.hpp"

RenderPipeline_State::RenderPipeline_State()
{
}

RenderPipeline_State::~RenderPipeline_State()
{
	for (std::vector<RenderPipeline_StateSetting*>::iterator iter = Settings.begin(); iter != Settings.end(); iter++)
	{
		delete *iter;
	}	
}

RenderPipeline_Mesh::RenderPipeline_Mesh()
	: Mesh_ID(-1)
{
}

RenderPipeline_Mesh::~RenderPipeline_Mesh()
{
	Renderer::Get()->Destroy_Mesh(Mesh_ID);	
}

RenderPipeline_Texture::RenderPipeline_Texture()
	: Texture(NULL)
	, RawTexture(NULL)
{
}

RenderPipeline_Texture::~RenderPipeline_Texture()
{
	//SAFE_DELETE(Texture);
	SAFE_DELETE(RawTexture);
}

RenderPipeline_Target::RenderPipeline_Target()
	: Target(NULL)
{
}

RenderPipeline_Target::~RenderPipeline_Target()
{
	SAFE_DELETE(Target);
	Attached_Textures.clear();
}

RenderPipeline_Shader::RenderPipeline_Shader()
	: Vertex_Shader(NULL)
	, Fragment_Shader(NULL)
	, Shader_Program(NULL)
{
}

RenderPipeline_ShaderUniform::RenderPipeline_ShaderUniform()
	: Texture(NULL)
{
}

RenderPipeline_Shader::~RenderPipeline_Shader()
{
	for (std::vector<RenderPipeline_ShaderUniform*>::iterator iter = Uniforms.begin(); iter != Uniforms.end(); iter++)
	{
		delete *iter;
	}	

	SAFE_DELETE(Shader_Program);
	SAFE_DELETE(Vertex_Shader);
	SAFE_DELETE(Fragment_Shader);
}

RenderPipeline_Pass::RenderPipeline_Pass()
	: Shader(NULL)
	, Target(NULL)
	, Mesh(NULL)
	, State(NULL)
	, Foreach(RenderPipeline_PassForEachType::None)
{
}

RenderPipeline_Pass::~RenderPipeline_Pass()
{
	for (std::vector<RenderPipeline_PassOutput*>::iterator iter = Outputs.begin(); iter != Outputs.end(); iter++)
	{
		delete *iter;
	}	
	for (std::vector<RenderPipeline_Pass*>::iterator iter = SubPasses.begin(); iter != SubPasses.end(); iter++)
	{
		delete *iter;
	}

	SAFE_DELETE(State);
}

RenderPipeline_Slot::RenderPipeline_Slot()
	: Shader(NULL)
	, Name("")
	, NameHash(0)
	, SortType(RenderPipeline_SlotSortType::None)
{
}

RenderPipeline_Slot::~RenderPipeline_Slot()
{
}

bool RenderPipeline_Slot::Sort_Front_To_Back(Drawable* a, Drawable* b)
{
	/*Vector3 camera_position = RenderPipeline::Get()->Get_Active_Camera()->Get_Position();

	float distance_a = (camera_position - a->Get_Position()).Length_Squared();
	float distance_b = (camera_position - b->Get_Position()).Length_Squared();

	return (distance_a < distance_b);*/

	return true;
}

RenderPipeline_PassOutput::RenderPipeline_PassOutput()
	: Texture(NULL)
{
}

RenderPipeline::RenderPipeline(Renderer* renderer)
	: m_renderer(renderer)
	, m_default_state(NULL)
	, m_active_camera(NULL)
	, m_active_light(NULL)
	, m_binded_shader_program(NULL)
{
}

RenderPipeline::~RenderPipeline()
{
	Reset();
}

void RenderPipeline::Reset()
{
	SAFE_DELETE(m_default_state);

	Reset_Reload_Trigger_Files();

	for (std::vector<RenderPipeline_Mesh*>::iterator iter = m_meshes.begin(); iter != m_meshes.end(); iter++)
	{
		delete *iter;
	}	
	for (std::vector<RenderPipeline_Texture*>::iterator iter = m_textures.begin(); iter != m_textures.end(); iter++)
	{
		delete *iter;
	}	
	for (std::vector<RenderPipeline_Target*>::iterator iter = m_targets.begin(); iter != m_targets.end(); iter++)
	{
		delete *iter;
	}
	for (std::vector<RenderPipeline_Shader*>::iterator iter = m_shaders.begin(); iter != m_shaders.end(); iter++)
	{
		delete *iter;
	}
	for (std::vector<RenderPipeline_Pass*>::iterator iter = m_passes.begin(); iter != m_passes.end(); iter++)
	{
		delete *iter;
	}
	for (std::vector<RenderPipeline_Slot*>::iterator iter = m_slots.begin(); iter != m_slots.end(); iter++)
	{
		delete *iter;
	}
	
	m_slots.clear();
	m_meshes.clear();
	m_textures.clear();
	m_targets.clear();
	m_shaders.clear();
	m_passes.clear();
}

bool RenderPipeline::Load_Config(const char* path)
{
	Stream* stream = StreamFactory::Open(path, StreamMode::Read);
	if (stream == NULL)
	{
		return false;
	}

	// Load source in a single string.
	int source_len = stream->Length();

	char* buffer = new char[source_len + 1];
	buffer[source_len] = '\0';
	DBG_ASSERT(buffer != NULL);

	stream->Read(buffer, 0, source_len);

	// Try and parse XML.
	rapidxml::xml_document<>* document = new rapidxml::xml_document<>();
	try
	{
		document->parse<0>(buffer);
	}
	catch (rapidxml::parse_error error)
	{
		int offset = error.where<char>() - buffer;
		int line = 0;
		int column = 0;

		StringHelper::Find_Line_And_Column(buffer, offset, line, column);

		DBG_LOG("Failed to parse render pipeline XML with error @ %i:%i: %s", line, column, error.what());
		delete stream;
		delete buffer;
		return false;
	}
	
	// Unload old state.
	Reset();
	
	// Store config path for reloading later on.
	m_config_path = path;
	Add_Reload_Trigger_File(path);

	// Find root node.	
	rapidxml::xml_node<>* root = document->first_node("xml", 0, false);
	DBG_ASSERT(root != NULL);

	// Load in defaults first.
	rapidxml::xml_node<>* defaults = root->first_node("defaults", 0, false);
	if (defaults != NULL)
	{
		Load_Defaults(defaults);
	}

	// Load in meshes.
	rapidxml::xml_node<>* meshes = root->first_node("meshes", 0, false);
	if (meshes != NULL)
	{
		Load_Meshes(meshes);
	}

	// Load in textures.
	rapidxml::xml_node<>* textures = root->first_node("textures", 0, false);
	if (textures != NULL)
	{
		Load_Textures(textures);
	}
	
	// Load in render targets.
	rapidxml::xml_node<>* targets = root->first_node("targets", 0, false);
	if (targets != NULL)
	{
		Load_Targets(targets);
	}
	
	// Load in shaders.
	rapidxml::xml_node<>* shaders = root->first_node("shaders", 0, false);
	if (shaders != NULL)
	{
		Load_Shaders(shaders);
	}
	
	// Load in passes.
	rapidxml::xml_node<>* passes = root->first_node("passes", 0, false);
	if (passes != NULL)
	{
		Load_Passes(passes);
	}

	// Load in slots.
	rapidxml::xml_node<>* slots = root->first_node("slots", 0, false);
	if (passes != NULL)
	{
		Load_Slots(slots);
	}

	// Clean up and return.
	delete stream;
	delete buffer;
	return true;
}

Camera* RenderPipeline::Get_Active_Camera()
{
	return m_active_camera;
}

void RenderPipeline::Set_Active_Camera(Camera* camera)
{	
	m_active_camera = camera;

	m_renderer->Set_Projection_Matrix(camera->Get_Projection_Matrix());
	m_renderer->Set_View_Matrix(camera->Get_View_Matrix());
	m_renderer->Set_World_Matrix(Matrix4::Identity());

	//m_renderer->Set_Viewport(camera->Get_Viewport());
}

Light* RenderPipeline::Get_Active_Light()
{
	return m_active_light;
}

RenderPipeline_Texture* RenderPipeline::Get_Texture_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Texture*>::iterator iter = m_textures.begin(); iter != m_textures.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

RenderPipeline_Mesh* RenderPipeline::Get_Mesh_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Mesh*>::iterator iter = m_meshes.begin(); iter != m_meshes.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

RenderPipeline_Target* RenderPipeline::Get_Target_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Target*>::iterator iter = m_targets.begin(); iter != m_targets.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

RenderPipeline_Shader* RenderPipeline::Get_Shader_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Shader*>::iterator iter = m_shaders.begin(); iter != m_shaders.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

RenderPipeline_Pass* RenderPipeline::Get_Pass_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Pass*>::iterator iter = m_passes.begin(); iter != m_passes.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

RenderPipeline_Slot* RenderPipeline::Get_Slot_From_Name(const char* name)
{
	for (std::vector<RenderPipeline_Slot*>::iterator iter = m_slots.begin(); iter != m_slots.end(); iter++)
	{
		if (stricmp((*iter)->Name.c_str(), name) == 0)
		{
			return *iter;
		}
	}
	return NULL;
}

void RenderPipeline::Draw(const FrameTime& time)
{
	Display* display = Display::Get();

	// Clear back buffer.
	m_renderer->Set_Viewport(Rect(0, 0, display->Get_Width(), display->Get_Height())); 
	m_renderer->Clear_Buffer();

	// Draw the scene/UI.
	UIManager* ui = GameEngine::Get()->Get_UIManager();
	Set_Active_Camera(ui->Get_Camera());
	ui->Draw(time);

	// Flip buffers.
	m_renderer->Flip(time);
}

void RenderPipeline::Draw_Game(const FrameTime& time)
{
	Display* display = Display::Get();
	Rect full_viewport = Rect(0, 0, display->Get_Width(), display->Get_Height());

	// Apply the default state.
	if (m_default_state != NULL)
	{
		Apply_State(time, m_default_state);
	}

	// Draw every pass!
	for (std::vector<RenderPipeline_Pass*>::iterator iter = m_passes.begin(); iter != m_passes.end(); iter++)
	{
		Draw_Pass(time, *iter);
	}

	// Remove some temp state variables we no longer need.
	m_active_light = NULL;

	// Reset some stuff.
	m_renderer->Set_Viewport(full_viewport);
}

void RenderPipeline::Apply_State(const FrameTime& time, RenderPipeline_State* state)
{
	Display* display = Display::Get();
	Rect full_viewport = Rect(0, 0, display->Get_Width(), display->Get_Height());
	Rect camera_viewport = m_active_camera->Get_Viewport();

	for (std::vector<RenderPipeline_StateSetting*>::iterator iter = state->Settings.begin(); iter != state->Settings.end(); iter++)
	{
		RenderPipeline_StateSetting* setting = *iter;
		switch (setting->Type)
		{
		case RenderPipeline_StateSettingType::ClearColorValue:	m_renderer->Set_Clear_Color(setting->ColorValue);		break;
		case RenderPipeline_StateSettingType::ClearDepthValue:	m_renderer->Set_Clear_Depth(setting->FloatValue);		break;
		case RenderPipeline_StateSettingType::CullFace:			m_renderer->Set_Cull_Face(setting->EnumValue);			break;
		case RenderPipeline_StateSettingType::DepthFunction:	m_renderer->Set_Depth_Function(setting->EnumValue);		break;
		case RenderPipeline_StateSettingType::DepthTest:		m_renderer->Set_Depth_Test(setting->BoolValue);			break;
		case RenderPipeline_StateSettingType::AlphaTest:		m_renderer->Set_Alpha_Test(setting->BoolValue);			break;
		case RenderPipeline_StateSettingType::Clear:			
			{
				if (setting->BoolValue == true)
				{
					m_renderer->Clear_Buffer(true, false);							
				}
				break;	
			}
		case RenderPipeline_StateSettingType::ClearDepth:			
			{
				if (setting->BoolValue == true)
				{
					m_renderer->Clear_Buffer(false, true);							
				}
				break;	
			}

		case RenderPipeline_StateSettingType::Blend:			m_renderer->Set_Blend(setting->BoolValue);				break;
		case RenderPipeline_StateSettingType::BlendFunction:	m_renderer->Set_Blend_Function(setting->EnumValue);		break;
		case RenderPipeline_StateSettingType::Viewport:
			{
				switch (setting->EnumValue)
				{
				case RendererOption::Output:					m_renderer->Set_Viewport(camera_viewport);				break;
				case RendererOption::Full:						m_renderer->Set_Viewport(full_viewport);				break;
				default:										DBG_ASSERT(false); 
				}
				break;
			}
		default:												DBG_ASSERT(false); 
		}
	}
}

void RenderPipeline::Apply_Shader(const FrameTime& time, RenderPipeline_Shader* shader)
{
	if (shader != m_binded_shader_program)
	{
		ShaderProgram* prog = shader == NULL ? NULL : shader->Shader_Program;
		m_renderer->Bind_Shader_Program(prog);
		m_binded_shader_program = shader;
	}
}

void RenderPipeline::Update_Shader_Uniforms()
{
	Matrix4 world_matrix					= m_renderer->Get_World_Matrix();
	Matrix4 view_matrix						= m_renderer->Get_View_Matrix();
	Matrix4 projection_matrix				= m_renderer->Get_Projection_Matrix();
	Matrix4 world_view_matrix				= view_matrix * world_matrix;
	Matrix4 world_view_projection_matrix	= projection_matrix * world_view_matrix;
	Matrix4 normal_matrix					= world_view_matrix.Inverse().Transpose();

	Matrix4 light_projection_matrix;
	Matrix4 light_view_matrix;

	if (m_active_light != NULL && m_active_light->Get_Shadow_Caster() == true)
	{
		light_projection_matrix = m_active_light->Get_Projection_Matrix();
		light_view_matrix = m_active_light->Get_View_Matrix();
	}

	Matrix4 light_world_view_projection_matrix	= light_projection_matrix * (light_view_matrix * world_matrix);

	Material* material						= m_renderer->Get_Material();

	int display_width						= (int)m_active_camera->Get_Viewport().Width;
	int display_height						= (int)m_active_camera->Get_Viewport().Height;

	int binded_texture_index				= 0 ;

	if (m_binded_shader_program != NULL && m_binded_shader_program->Shader_Program != NULL)
	{
		ShaderProgram* prog = m_binded_shader_program->Shader_Program;

		for (std::vector<RenderPipeline_ShaderUniform*>::iterator iter = m_binded_shader_program->Uniforms.begin(); iter != m_binded_shader_program->Uniforms.end(); iter++)
		{
			RenderPipeline_ShaderUniform* uniform = *iter;
			switch (uniform->Type)
			{
			// Samplers
			case RenderPipeline_ShaderUniformType::Texture:								
				{
					const Texture* tex = uniform->Texture->Texture != NULL ? uniform->Texture->Texture->Get() : uniform->Texture->RawTexture;

					m_renderer->Bind_Texture(tex, binded_texture_index);
					prog->Bind_Texture(uniform->Name.c_str(), binded_texture_index);
					binded_texture_index++;
					break;
				}
			case RenderPipeline_ShaderUniformType::MaterialTexture:			
				{
					if (material != NULL)
					{
						//DBG_ASSERT(material != NULL && material->Get_Texture() != NULL);
						m_renderer->Bind_Texture(material->Get_Texture()->Get(), binded_texture_index);
						prog->Bind_Texture(uniform->Name.c_str(), binded_texture_index);
						binded_texture_index++;
					}
					break;
				}

			// Floats
			case RenderPipeline_ShaderUniformType::CameraNearClip:						prog->Bind_Float(uniform->Name.c_str(), m_active_camera->Get_Near_Clip());							break;
			case RenderPipeline_ShaderUniformType::CameraFarClip:						prog->Bind_Float(uniform->Name.c_str(), m_active_camera->Get_Far_Clip());							break;
			case RenderPipeline_ShaderUniformType::CameraFOV:							prog->Bind_Float(uniform->Name.c_str(), m_active_camera->Get_FOV());								break;
			case RenderPipeline_ShaderUniformType::MaterialShininess:					prog->Bind_Float(uniform->Name.c_str(), material == NULL ? 0.0f : material->Get_Shininess());		break;
			case RenderPipeline_ShaderUniformType::LightRadius:							prog->Bind_Float(uniform->Name.c_str(), m_active_light->Get_Radius());								break;
			case RenderPipeline_ShaderUniformType::LightOuterRadius:					prog->Bind_Float(uniform->Name.c_str(), m_active_light->Get_Outer_Radius());						break;

			// Ints
			case RenderPipeline_ShaderUniformType::LightType:							prog->Bind_Int(uniform->Name.c_str(), m_active_light->Get_Type());									break;

			// Vector3's
			case RenderPipeline_ShaderUniformType::MaterialSpecular:					prog->Bind_Vector(uniform->Name.c_str(), material == NULL ? Vector3(0.f, 0.f, 0.f) : material->Get_Specular());					break;
			case RenderPipeline_ShaderUniformType::CameraPosition:						prog->Bind_Vector(uniform->Name.c_str(), m_active_camera->Get_Position());										break;
			case RenderPipeline_ShaderUniformType::Resolution:							prog->Bind_Vector(uniform->Name.c_str(), Vector3((float)display_width, (float)display_height, 1.0f));			break;
			case RenderPipeline_ShaderUniformType::LightPosition:						prog->Bind_Vector(uniform->Name.c_str(), world_view_matrix * Vector4(m_active_light->Get_Position(), 1.0f));	break;
			case RenderPipeline_ShaderUniformType::LightDirection:						prog->Bind_Vector(uniform->Name.c_str(), normal_matrix * Vector4(m_active_light->Get_Direction(), 1.0f));		break;
			case RenderPipeline_ShaderUniformType::LightColor:							prog->Bind_Vector(uniform->Name.c_str(), m_active_light->Get_Color().To_Vector4());								break;

			// Matrix4's
			case RenderPipeline_ShaderUniformType::WorldMatrix:							prog->Bind_Matrix(uniform->Name.c_str(), world_matrix);								break;
			case RenderPipeline_ShaderUniformType::ViewMatrix:							prog->Bind_Matrix(uniform->Name.c_str(), view_matrix);								break;
			case RenderPipeline_ShaderUniformType::ProjectionMatrix:					prog->Bind_Matrix(uniform->Name.c_str(), projection_matrix);						break;
			case RenderPipeline_ShaderUniformType::WorldViewMatrix:						prog->Bind_Matrix(uniform->Name.c_str(), world_view_matrix);						break;
			case RenderPipeline_ShaderUniformType::WorldViewProjectionMatrix:			prog->Bind_Matrix(uniform->Name.c_str(), world_view_projection_matrix);				break;
			case RenderPipeline_ShaderUniformType::NormalMatrix:						prog->Bind_Matrix(uniform->Name.c_str(), normal_matrix);							break;
			case RenderPipeline_ShaderUniformType::InverseWorldMatrix:					prog->Bind_Matrix(uniform->Name.c_str(), world_matrix.Inverse());					break;
			case RenderPipeline_ShaderUniformType::InverseViewMatrix:					prog->Bind_Matrix(uniform->Name.c_str(), view_matrix.Inverse());					break;
			case RenderPipeline_ShaderUniformType::InverseProjectionMatrix:				prog->Bind_Matrix(uniform->Name.c_str(), projection_matrix.Inverse());				break;
			case RenderPipeline_ShaderUniformType::InverseWorldViewMatrix:				prog->Bind_Matrix(uniform->Name.c_str(), world_view_matrix.Inverse());				break;
			case RenderPipeline_ShaderUniformType::InverseWorldViewProjectionMatrix:	prog->Bind_Matrix(uniform->Name.c_str(), world_view_projection_matrix.Inverse());	break;
			case RenderPipeline_ShaderUniformType::InverseNormalMatrix:					prog->Bind_Matrix(uniform->Name.c_str(), normal_matrix.Inverse());					break;
			case RenderPipeline_ShaderUniformType::LightWorldViewProjectionMatrix:		prog->Bind_Matrix(uniform->Name.c_str(), light_world_view_projection_matrix);		break;
			case RenderPipeline_ShaderUniformType::LightProjectionMatrix:				prog->Bind_Matrix(uniform->Name.c_str(), light_projection_matrix);					break;
			case RenderPipeline_ShaderUniformType::LightViewMatrix:						prog->Bind_Matrix(uniform->Name.c_str(), light_view_matrix);						break;

			// Special ones!
			case RenderPipeline_ShaderUniformType::TextureSize:
				{
					prog->Bind_Vector(uniform->Name.c_str(), Vector3((float)uniform->Texture->Texture->Get()->Get_Width(), (float)uniform->Texture->Texture->Get()->Get_Height(), 0.0f));	break;
					break;
				}

			default: DBG_ASSERT(false); 
			}
		}
	}
}

void RenderPipeline::Apply_Outputs(const FrameTime& time, RenderPipeline_Pass* pass)
{
	std::vector<OutputBufferType::Type> outputs;
	
	for (std::vector<RenderPipeline_PassOutput*>::iterator iter = pass->Outputs.begin(); iter != pass->Outputs.end(); iter++)
	{
		RenderPipeline_PassOutput* output = *iter;
		switch (output->Type)
		{
		case RenderPipeline_PassOutputType::BackBuffer:
			{
				outputs.push_back(OutputBufferType::BackBuffer);
				break;
			}
		case RenderPipeline_PassOutputType::Texture:
			{
				int texture_index = -1;
				DBG_ASSERT(pass->Target != NULL);

				int index = 0;
				for (std::vector<RenderPipeline_Texture*>::iterator iter = pass->Target->Attached_Textures.begin(); iter != pass->Target->Attached_Textures.end(); iter++, index++)
				{
					RenderPipeline_Texture* tex = *iter;
					if (tex == output->Texture)
					{
						texture_index = index;
						break;
					}
				}

				DBG_ASSERT(texture_index >= 0);
				outputs.push_back((OutputBufferType::Type)(OutputBufferType::RenderTargetTexture0 + texture_index));

				break;
			}
		default:
			{
				DBG_ASSERT(false);
				break;
			}
		}
	}
	
	m_renderer->Set_Output_Buffers(outputs);
	//m_renderer->Set_Viewport(m_active_camera->Get_Viewport());
}

void RenderPipeline::Draw_Scene(const FrameTime& time)
{
	// Calculate matricies.
	Matrix4 projection_matrix = m_active_camera->Get_Projection_Matrix();
	Matrix4 view_matrix       = m_active_camera->Get_View_Matrix();
	Matrix4 world_matrix	  = Matrix4::Identity();

	Draw_Scene_With_Matrices(time, projection_matrix, view_matrix, world_matrix);
}

void RenderPipeline::Draw_Scene_With_Matrices(const FrameTime& time, Matrix4& projection_matrix, Matrix4& view_matrix, Matrix4& world_matrix)
{
	// Shader uniforms will need updating!
	m_renderer->Set_Projection_Matrix(projection_matrix);
	m_renderer->Set_View_Matrix(view_matrix);
	m_renderer->Set_World_Matrix(world_matrix);

	// Render all drawables.
	std::vector<Drawable*>& drawable = GameEngine::Get()->Get_Scene()->Get_Drawables();
	int counter = 0;
	for (auto iter = m_slots.begin(); iter != m_slots.end(); iter++)
	{
		RenderPipeline_Slot* slot = *iter;

		// Do we render this slot on this pass?
		bool on_pass = false;
		for (auto passiter = slot->Passes.begin(); passiter != slot->Passes.end(); passiter++)
		{
			if (*passiter == m_current_pass)
			{
				on_pass = true;
				break;
			}
		}
		if (on_pass == false)
			continue;

		// Gather all drawables in this slot (we should really do this as and when things are added to slots >_>).
		std::vector<Drawable*> slot_contents;
		for (auto iter2 = drawable.begin(); iter2 != drawable.end(); iter2++)
		{
			Drawable* drawable = *iter2;
			Camera* draw_camera = drawable->Get_Draw_Camera();

			if (drawable->Get_Render_Slot_Hash() == slot->NameHash &&
				(draw_camera == NULL || draw_camera == m_active_camera))
			{
				slot_contents.push_back(drawable);
			}
		}

		// Sort slot contents.
		switch (slot->SortType)
		{
		case RenderPipeline_SlotSortType::Front_To_Back:
			// TODO: Implement this stuff.
			//std::sort(slot_contents.begin(), slot_contents.end(), RenderPipeline_Slot::Sort_Front_To_Back);
			break;
		}

		// Apply shader.
		Apply_Shader(time, slot->Shader);
		Update_Shader_Uniforms();

		// Render contents.
		for (auto iter2 = slot_contents.begin(); iter2 != slot_contents.end(); iter2++)
		{
			Drawable* drawable = *iter2;
			drawable->Draw(time, this);
		}
	}

	/*
	// Render all lights.
	std::vector<Light*>& lights = GameEngine::Get()->Get_Scene()->Get_Lights();
	for (auto iter = lights.begin(); iter != lights.end(); iter++)
	{
		Light* light = *iter;

		// Move to lights position.
		m_renderer->Set_World_Matrix(world_matrix * Matrix4::Translate(light->Get_Position()));
		Update_Shader_Uniforms();

		// Draw sphere.
		m_renderer->Draw_Wireframe_Sphere(0.1f);
		m_renderer->Draw_Arrow(light->Get_Direction());
	}
	*/
}

void RenderPipeline::Draw_Pass(const FrameTime& time, RenderPipeline_Pass* pass)
{
	// Not enabled? :(
	if (pass->Enabled == false)
	{
		return;
	}

	// Store current pass.
	m_current_pass = pass;

	// Attach target.
	if (pass->Target != NULL)
	{
		m_renderer->Bind_Render_Target(pass->Target->Target);		
	}
	else
	{
		m_renderer->Bind_Render_Target(NULL);					
	}
	m_renderer->Bind_Texture(NULL, 0); 

	// Attach shader.
	Apply_Shader(time, pass->Shader);

	// Setup outputs.
	Apply_Outputs(time, pass);
		
	// Apply default state.
	if (pass->State != NULL)
	{
		Apply_State(time, pass->State);
	}	

	// Straight-pass?
	if (pass->Foreach == RenderPipeline_PassForEachType::None)
	{
		// Time to do some actual rendering!
		if (pass->Type == RenderPipeline_PassType::Mesh)
		{
			DBG_ASSERT(pass->Mesh != NULL);
			Update_Shader_Uniforms();
			m_renderer->Render_Mesh(pass->Mesh->Mesh_ID);
		}
		else if (pass->Type == RenderPipeline_PassType::Scene)
		{
			Draw_Scene(time);
		}
		else
		{
			DBG_ASSERT(false);
		}

		// Do any defined sub-passes.
		for (std::vector<RenderPipeline_Pass*>::iterator passiter = pass->SubPasses.begin(); passiter != pass->SubPasses.end(); passiter++)
		{
			Draw_Pass(time, *passiter);
		}
	}

	// Enumerating over lights.
	else if (pass->Foreach == RenderPipeline_PassForEachType::Light)
	{
		std::vector<Light*> lights = GameEngine::Get()->Get_Scene()->Get_Lights();
	
		for (std::vector<Light*>::iterator iter = lights.begin(); iter != lights.end(); iter++)
		{
			m_active_light = *iter;

			for (std::vector<RenderPipeline_Pass*>::iterator passiter = pass->SubPasses.begin(); passiter != pass->SubPasses.end(); passiter++)
			{
				Draw_Pass(time, *passiter);
				m_current_pass = pass;
			}
		}
	}
	
	// Enumerating over shadow casting lights.
	else if (pass->Foreach == RenderPipeline_PassForEachType::Shadow_Casting_Light)
	{
		std::vector<Light*> lights = GameEngine::Get()->Get_Scene()->Get_Lights();
	
		for (std::vector<Light*>::iterator iter = lights.begin(); iter != lights.end(); iter++)
		{
			m_active_light = *iter;

			if (m_active_light->Get_Shadow_Caster() == true)
			{
				for (std::vector<RenderPipeline_Pass*>::iterator passiter = pass->SubPasses.begin(); passiter != pass->SubPasses.end(); passiter++)
				{
					Draw_Pass(time, *passiter);
					m_current_pass = pass;
				}
			}
		}
	}
	
	// Wut
	else
	{
		DBG_ASSERT(false);
	}
}

void RenderPipeline::Reload()
{
	Load_Config(m_config_path.c_str());
}

std::string RenderPipeline::Get_Attribute_Value(rapidxml::xml_node<>* node, const char* name, const char* def)
{
	rapidxml::xml_attribute<>* sub = node->first_attribute(name, 0, false);
	if (sub != NULL)
	{
		return sub->value();
	}
	else
	{
		if (def == NULL)
		{
			DBG_ASSERT_STR(false, "Missing attribute '%s' in node '%s'.", name, node->name());
		}
		else
		{
			return def;
		}
	}

	return "";
}

std::string RenderPipeline::Get_Node_Value(rapidxml::xml_node<>* node, const char* name, const char* def)
{
	rapidxml::xml_node<>* sub = node->first_node(name, 0, false);
	if (sub != NULL)
	{
		return sub->value();
	}
	else
	{
		if (def == NULL)
		{
			DBG_ASSERT_STR(false, "Missing sub-node '%s' in node '%s'.", name, node->name());
		}
		else
		{
			return def;
		}
	}

	return "";
}

void RenderPipeline::Load_Defaults(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* state_node = parent_node->first_node("state", 0, false);
	if (state_node != NULL)
	{
		m_default_state = Load_State(state_node);
		DBG_LOG("Loading default state.");
	}
}

void RenderPipeline::Load_Meshes(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("mesh", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Mesh* mesh = Load_Mesh(node);
		m_meshes.push_back(mesh);
			
		node = node->next_sibling("mesh", 0, false);
		DBG_LOG("Loaded pipeline mesh: %s", mesh->Name.c_str());
	}
}

void RenderPipeline::Load_Textures(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("texture", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Texture* texture = Load_Texture(node);
		m_textures.push_back(texture);
			
		node = node->next_sibling("texture", 0, false);
		DBG_LOG("Loaded pipeline texture: %s", texture->Name.c_str());
	}
}

void RenderPipeline::Load_Targets(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("target", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Target* target = Load_Target(node);
		m_targets.push_back(target);
			
		node = node->next_sibling("target", 0, false);
		DBG_LOG("Loaded pipeline target: %s", target->Name.c_str());
	}
}

void RenderPipeline::Load_Shaders(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("shader", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Shader* shader = Load_Shader(node);
		m_shaders.push_back(shader);
			
		node = node->next_sibling("shader", 0, false);
		DBG_LOG("Loaded pipeline shader: %s", shader->Name.c_str());
	}
}

void RenderPipeline::Load_Passes(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("pass", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Pass* pass = Load_Pass(node);
		m_passes.push_back(pass);
			
		node = node->next_sibling("pass", 0, false);
		DBG_LOG("Loaded pipeline pass: %s", pass->Name.c_str());
	}
}

void RenderPipeline::Load_Slots(rapidxml::xml_node<>* parent_node)
{
	rapidxml::xml_node<>* node = parent_node->first_node("slot", 0, false);
	while (node != NULL)
	{
		RenderPipeline_Slot* slot = Load_Slot(node);
		m_slots.push_back(slot);
			
		node = node->next_sibling("slot", 0, false);
		DBG_LOG("Loaded pipeline render slot: %s", slot->Name.c_str());
	}
}

RenderPipeline_State* RenderPipeline::Load_State(rapidxml::xml_node<>* node)
{
	RenderPipeline_State* state = new RenderPipeline_State();

	rapidxml::xml_node<>* value_node = node->first_node(NULL);
	while (value_node != NULL)
	{
		RenderPipeline_StateSetting* state_value = new RenderPipeline_StateSetting();
		std::string name  = value_node->name();	
		std::string value = value_node->value();
		
		// Work out type of state.
		bool found = false;
		const char* value_type = "";

#define STATE_SETTING(setting_name, setting_enum, setting_value_type)					\
			if (stricmp(name.c_str(), setting_name) == 0)								\
			{																			\
				state_value->Type = RenderPipeline_StateSettingType::##setting_enum##;	\
				value_type = setting_value_type;										\
				found = true;															\
			} 
#include "Engine\Renderer\RenderPipeline_StateSettingType.inc"
#undef STATE_SETTING
		
		if (found == false)
		{
			DBG_ASSERT_STR(false, "Invalid state setting '%s'.", name.c_str());
		}
		
		// Parse value.
		if (stricmp("color", value_type) == 0)
		{
			state_value->ValueType = RenderPipeline_StateSettingValueType::Color;
			DBG_ASSERT_STR(Color::Parse(value.c_str(), state_value->ColorValue), "Could not parse color value in state setting '%s'.", name.c_str());
		}
		else if (stricmp("float", value_type) == 0)
		{
			state_value->ValueType = RenderPipeline_StateSettingValueType::Float;
			state_value->FloatValue = (float)atof(value.c_str());
		}
		else if (stricmp("int", value_type) == 0)
		{
			state_value->ValueType = RenderPipeline_StateSettingValueType::Int;
			state_value->IntValue = atoi(value.c_str());
		}
		else if (stricmp("bool", value_type) == 0)
		{
			state_value->ValueType = RenderPipeline_StateSettingValueType::Bool;
			state_value->BoolValue = (stricmp(value.c_str(), "false") == 0 || stricmp(value.c_str(), "0") == 0 ? false : true);
		}
		else if (stricmp("enum", value_type) == 0)
		{
			found = false;
			
			state_value->ValueType = RenderPipeline_StateSettingValueType::Enum;

#define STATE_SETTING(setting_name, setting_enum)														\
				if (stricmp(value.c_str(), setting_name) == 0)											\
				{																						\
					state_value->EnumValue = RendererOption::##setting_enum##;	\
					found = true;																		\
				} 
#include "Engine\Renderer\Renderer_RenderOptions.inc"
#undef STATE_SETTING

			if (found == false)
			{
				DBG_ASSERT_STR(false, "Invalid value '%s' for state setting '%s'.", value.c_str(), name.c_str());
			}
		}
		else
		{
			DBG_ASSERT_STR(false, "Invalid internal value type for setting '%s'.", name.c_str());
		}

		state->Settings.push_back(state_value);

		value_node = value_node->next_sibling(NULL);
	}

	return state;
}

RenderPipeline_Mesh* RenderPipeline::Load_Mesh(rapidxml::xml_node<>* node)
{
	RenderPipeline_Mesh* state = new RenderPipeline_Mesh();
	state->Name = Get_Attribute_Value(node, "name", NULL);

	DBG_ASSERT_STR(Get_Mesh_From_Name(state->Name.c_str()) == NULL, "Duplicate mesh name '%s'.", state->Name.c_str());

	// Create mesh.
	std::string primitive_type = node->first_node("primative", 0, false)->value();
	if (stricmp(primitive_type.c_str(), "quads") == 0)
	{
		std::string verticies_string	= StringHelper::Remove_Whitespace(Get_Node_Value(node, "vertices",				NULL).c_str());
		std::string normals_string		= StringHelper::Remove_Whitespace(Get_Node_Value(node, "normals",				NULL).c_str());
		std::string texcoords_string	= StringHelper::Remove_Whitespace(Get_Node_Value(node, "texture-coordinates",	NULL).c_str());

		std::vector<std::string> vertices;
		std::vector<std::string> normals;
		std::vector<std::string> texcoords;

		StringHelper::Split(verticies_string.c_str(),	',', vertices);
		StringHelper::Split(normals_string.c_str(),		',', normals);
		StringHelper::Split(texcoords_string.c_str(),	',', texcoords);

		int quad_count = (int)ceil(vertices.size() / 12);
		int tri_count  = quad_count * 2;
		state->Mesh_ID = m_renderer->Start_Mesh(MeshPrimitiveType::Triangle, tri_count * 3, tri_count);
		
		DBG_ASSERT_STR(vertices.size()  == quad_count * 12, "Incorrect number of verticies in mesh '%s'.",				state->Name.c_str());
		DBG_ASSERT_STR(normals.size()   == quad_count * 12, "Incorrect number of normals in mesh '%s'.",				state->Name.c_str());
		DBG_ASSERT_STR(texcoords.size() == quad_count * 8,  "Incorrect number of texture coordinates in mesh '%s'.",	state->Name.c_str());

		for (int vi = 0, ni = 0, ti = 0; vi < (int)vertices.size(); vi += 12, ni += 12, ti += 8)
		{
			Vector3 p1 = Vector3(
							(float)atof(vertices.at(vi + 0).c_str()),							
							(float)atof(vertices.at(vi + 1).c_str()),							
						 	(float)atof(vertices.at(vi + 2).c_str())
						 );
			Vector3 p2 = Vector3(
							(float)atof(vertices.at(vi + 3).c_str()),							
							(float)atof(vertices.at(vi + 4).c_str()),							
						 	(float)atof(vertices.at(vi + 5).c_str())
						 );
			Vector3 p3 = Vector3(
							(float)atof(vertices.at(vi + 6).c_str()),							
							(float)atof(vertices.at(vi + 7).c_str()),							
						 	(float)atof(vertices.at(vi + 8).c_str())
						 );
			Vector3 p4 = Vector3(
							(float)atof(vertices.at(vi + 9).c_str()),							
							(float)atof(vertices.at(vi + 10).c_str()),							
						 	(float)atof(vertices.at(vi + 11).c_str())
						 );


			Vector3 n1 = Vector3(
							(float)atof(normals.at(vi + 0).c_str()),							
							(float)atof(normals.at(vi + 1).c_str()),							
						 	(float)atof(normals.at(vi + 2).c_str())
						 );
			Vector3 n2 = Vector3(
							(float)atof(normals.at(vi + 3).c_str()),							
							(float)atof(normals.at(vi + 4).c_str()),							
						 	(float)atof(normals.at(vi + 5).c_str())
						 );
			Vector3 n3 = Vector3(
							(float)atof(normals.at(vi + 6).c_str()),							
							(float)atof(normals.at(vi + 7).c_str()),							
						 	(float)atof(normals.at(vi + 8).c_str())
						 );
			Vector3 n4 = Vector3(
							(float)atof(normals.at(vi + 9).c_str()),							
							(float)atof(normals.at(vi + 10).c_str()),							
						 	(float)atof(normals.at(vi + 11).c_str())
						 );

			
			Vector3 uv1 = Vector3(
							(float)atof(texcoords.at(ti + 0).c_str()),							
							(float)atof(texcoords.at(ti + 1).c_str()),	
							0.0f
						 );
			Vector3 uv2 = Vector3(
							(float)atof(texcoords.at(ti + 2).c_str()),							
							(float)atof(texcoords.at(ti + 3).c_str()),	
							0.0f
						 );
			Vector3 uv3 = Vector3(
							(float)atof(texcoords.at(ti + 4).c_str()),							
							(float)atof(texcoords.at(ti + 5).c_str()),	
							0.0f
						 );
			Vector3 uv4 = Vector3(
							(float)atof(texcoords.at(ti + 6).c_str()),							
							(float)atof(texcoords.at(ti + 7).c_str()),	
							0.0f
						 );

			int v1 = m_renderer->Add_Mesh_Vertex(state->Mesh_ID, p1, n1, 1.0f, 1.0f, 1.0f, 1.0f, uv1.X, uv1.Y);
			int v2 = m_renderer->Add_Mesh_Vertex(state->Mesh_ID, p2, n2, 1.0f, 1.0f, 1.0f, 1.0f, uv2.X, uv2.Y);
			int v3 = m_renderer->Add_Mesh_Vertex(state->Mesh_ID, p3, n3, 1.0f, 1.0f, 1.0f, 1.0f, uv3.X, uv3.Y);
			int v4 = m_renderer->Add_Mesh_Vertex(state->Mesh_ID, p4, n4, 1.0f, 1.0f, 1.0f, 1.0f, uv4.X, uv4.Y);

			m_renderer->Add_Mesh_Primitive(state->Mesh_ID, v1, v2, v4);
			m_renderer->Add_Mesh_Primitive(state->Mesh_ID, v2, v3, v4);
		}

		m_renderer->End_Mesh(state->Mesh_ID);
	}
	else
	{
		DBG_ASSERT_STR(false, "Unsupported primitive type: '%s'", state->Name.c_str());
	}
	
	return state;
}

RenderPipeline_Texture*	RenderPipeline::Load_Texture(rapidxml::xml_node<>* node)
{
	RenderPipeline_Texture* state = new RenderPipeline_Texture();
	state->Name = Get_Attribute_Value(node, "name", NULL);
	
	DBG_ASSERT_STR(Get_Texture_From_Name(state->Name.c_str()) == NULL, "Duplicate texture name '%s'.", state->Name.c_str());

	std::string format_string = Get_Attribute_Value(node, "format", "");
	std::string width_string  = Get_Attribute_Value(node, "width",	"");
	std::string height_string = Get_Attribute_Value(node, "height", "");
	std::string flags_string  = Get_Attribute_Value(node, "flags",	"");
	std::string file_string   = Get_Attribute_Value(node, "file",	"");

	// Calculate texture flags.
	TextureFlags::Type flags;
	std::vector<std::string> semi_flags;
	StringHelper::Split(flags_string.c_str(), ',', semi_flags);

	for (std::vector<std::string>::iterator iter = semi_flags.begin(); iter != semi_flags.end(); iter++)
	{
		std::string& val = *iter;
		if (stricmp(val.c_str(), "AllowRepeat") == 0)
		{
			flags = (TextureFlags::Type)((int)flags | (int)TextureFlags::AllowRepeat);
		}
		else
		{			
			DBG_ASSERT_STR(false, "Invalid texture flag '%s' in node '%s'.", format_string.c_str(), state->Name.c_str());
		}
	}

	// Calculate texture format.
	TextureFormat::Type format;
	if (stricmp(format_string.c_str(), "R8G8B8A8") == 0)
	{
		format = TextureFormat::R8G8B8A8;
	}
	else if (stricmp(format_string.c_str(), "R8G8B8") == 0)
	{
		format = TextureFormat::R8G8B8;
	}
	else if (stricmp(format_string.c_str(), "R32FG32FB32FA32F") == 0)
	{
		format = TextureFormat::R32FG32FB32FA32F;
	}
	else if (stricmp(format_string.c_str(), "DepthFormat") == 0)
	{
		format = TextureFormat::DepthFormat;
	}
	else if (stricmp(format_string.c_str(), "StencilFormat") == 0)
	{
		format = TextureFormat::StencilFormat;
	}
	else if (format_string != "")
	{
		DBG_ASSERT_STR(false, "Invalid texture format '%s' in node '%s'.", format_string.c_str(), state->Name.c_str());
	}
	else if (file_string == "")
	{
		DBG_ASSERT_STR(false, "No texture format specified for node '%s'.", state->Name.c_str());
	}

	// Create and return texture.
	if (file_string != "")
	{
		state->Texture = TextureFactory::Load(file_string.c_str(), flags);
		Add_Reload_Trigger_File(file_string.c_str());

		DBG_ASSERT_STR(state->Texture != NULL,  "Failed to load texture '%s'.", file_string.c_str())
	}
	else
	{
		// Some constants.
		if (stricmp(width_string.c_str(), "DISPLAY_WIDTH") == 0)
		{
			width_string = StringHelper::To_String(Display::Get()->Get_Width());
		}
		if (stricmp(height_string.c_str(), "DISPLAY_HEIGHT") == 0)
		{
			height_string = StringHelper::To_String(Display::Get()->Get_Height());
		}

		DBG_ASSERT_STR(width_string != "",  "Invalid width '%s' for texture '%s'.", width_string.c_str(), state->Name.c_str())
		DBG_ASSERT_STR(height_string != "", "Invalid height '%s' for texture '%s'.", height_string.c_str(), state->Name.c_str())

		int width  = atoi(width_string.c_str());
		int height = atoi(height_string.c_str());

		state->RawTexture = m_renderer->Create_Texture(width, height, width, format, flags);
	}

	return state;
}

RenderPipeline_Target* RenderPipeline::Load_Target(rapidxml::xml_node<>* node)
{
	RenderPipeline_Target* state = new RenderPipeline_Target();
	state->Name = Get_Attribute_Value(node, "name", NULL);
	state->Target = m_renderer->Create_Render_Target();
	
	DBG_ASSERT_STR(Get_Target_From_Name(state->Name.c_str()) == NULL, "Duplicate target name '%s'.", state->Name.c_str());

	rapidxml::xml_node<>* value_node = node->first_node("attached-texture");
	while (value_node != NULL)
	{
		std::string name = Get_Attribute_Value(value_node, "name", NULL);
		std::string type = Get_Attribute_Value(value_node, "type", NULL);

		RenderPipeline_Texture* texture = Get_Texture_From_Name(name.c_str());
		DBG_ASSERT_STR(texture != NULL, "Missing texture '%s' in render target '%s'.", name.c_str(), state->Name.c_str());

		if (stricmp(type.c_str(), "color") == 0)
		{
			state->Target->Bind_Texture(RenderTargetBufferType::Color, texture->Texture != NULL ? texture->Texture->Get() : texture->RawTexture);
		}
		else if (stricmp(type.c_str(), "depth") == 0)		
		{
			state->Target->Bind_Texture(RenderTargetBufferType::Depth,  texture->Texture != NULL ? texture->Texture->Get() : texture->RawTexture);
		}
		else if (stricmp(type.c_str(), "stencil") == 0)	
		{
			state->Target->Bind_Texture(RenderTargetBufferType::Depth,  texture->Texture != NULL ? texture->Texture->Get() : texture->RawTexture);
		}
		else
		{
			DBG_ASSERT_STR(false, "Invalid type '%s' for attached-texture '%s' in render target '%s'.", type.c_str(), name.c_str(), state->Name.c_str());
		}

		state->Attached_Textures.push_back(texture);
		value_node = value_node->next_sibling("attached-texture");
	}

	state->Target->Validate();

	return state;
}

RenderPipeline_Shader* RenderPipeline::Load_Shader(rapidxml::xml_node<>* node)
{
	RenderPipeline_Shader* state = new RenderPipeline_Shader();
	state->Name = Get_Attribute_Value(node, "name", NULL);
	
	DBG_ASSERT_STR(Get_Shader_From_Name(state->Name.c_str()) == NULL, "Duplicate shader name '%s'.", state->Name.c_str());

	std::string vertex_shader_filename   = Get_Node_Value(node, "vertex", "");
	std::string fragment_shader_filename = Get_Node_Value(node, "fragment", "");

	// Load shaders.
	std::vector<Shader*> shaders;
	if (vertex_shader_filename != "")
	{
		state->Vertex_Shader = ShaderFactory::Load(vertex_shader_filename.c_str(), ShaderType::Vertex);
		DBG_ASSERT_STR(state->Vertex_Shader != NULL, "Failed to load vertex shader '%s'.", vertex_shader_filename.c_str());
		shaders.push_back(state->Vertex_Shader);

		Add_Reload_Trigger_File(vertex_shader_filename.c_str());
	}
	if (fragment_shader_filename != "")
	{
		state->Fragment_Shader = ShaderFactory::Load(fragment_shader_filename.c_str(), ShaderType::Fragment);
		DBG_ASSERT_STR(state->Fragment_Shader != NULL, "Failed to load fragment shader '%s'.", vertex_shader_filename.c_str());
		shaders.push_back(state->Fragment_Shader);

		Add_Reload_Trigger_File(fragment_shader_filename.c_str());
	}

	// Create shader program.
	state->Shader_Program = m_renderer->Create_Shader_Program(shaders);

	// Load uniforms.
	rapidxml::xml_node<>* uniforms_node = node->first_node("uniforms");
	if (uniforms_node != NULL)
	{		
		rapidxml::xml_node<>* value_node = uniforms_node->first_node("uniform");

		while (value_node != NULL)
		{
			std::string name	= Get_Attribute_Value(value_node, "name", NULL);
			std::string type	= Get_Attribute_Value(value_node, "type", NULL);
			std::string value	= Get_Attribute_Value(value_node, "value", "");
			std::string texture = Get_Attribute_Value(value_node, "texture", "");

			RenderPipeline_ShaderUniform* uniform = new RenderPipeline_ShaderUniform();
			uniform->Name  = name;
			uniform->Value = value;

			bool found = false;
#define SHADER_UNIFORM(uniform_type, uniform_name, uniform_constant)									\
			if (stricmp(type.c_str(), #uniform_type) == 0 && stricmp(value.c_str(), uniform_name) == 0)	\
			{																							\
				uniform->Type = RenderPipeline_ShaderUniformType::##uniform_constant##;					\
				found = true;																			\
			} 
#include "Engine\Renderer\RenderPipeline_ShaderUniformType.inc"
#undef SHADER_UNIFORM

			if (found == false)
			{	
				if (stricmp(type.c_str(), "texture") == 0)
				{					
					uniform->Type = RenderPipeline_ShaderUniformType::Texture;
					uniform->Texture = Get_Texture_From_Name(value.c_str());
					DBG_ASSERT_STR(uniform->Texture != NULL, "Failed to load uniform '%s' in shader '%s', missing texture with name '%s'.", name.c_str(), state->Name.c_str(), value.c_str());				
				}
				else
				{
					DBG_ASSERT_STR(false, "Failed to load uniform '%s' in shader '%s', invalid type '%s'.", name.c_str(), state->Name.c_str(), type.c_str());
				}
			}
			else
			{
				if (uniform->Type == RenderPipeline_ShaderUniformType::TextureSize)
				{
					uniform->Texture = Get_Texture_From_Name(texture.c_str());
					DBG_ASSERT_STR(uniform->Texture != NULL, "Failed to load uniform '%s' in shader '%s', missing texture with name '%s'.", name.c_str(), state->Name.c_str(), value.c_str());								
				}
			}
			
			state->Uniforms.push_back(uniform);

			value_node = value_node->next_sibling("uniform");
		}
	}

	return state;
}

RenderPipeline_Pass* RenderPipeline::Load_Pass(rapidxml::xml_node<>* node)
{
	std::string enabled_string = Get_Attribute_Value(node, "enabled", "");

	RenderPipeline_Pass* state = new RenderPipeline_Pass();
	state->Name		= Get_Attribute_Value(node, "name", NULL);
	state->Enabled	= (stricmp(enabled_string.c_str(), "0") == 0 || stricmp(enabled_string.c_str(), "false") == 0) ? false : true;

	DBG_ASSERT_STR(Get_Pass_From_Name(state->Name.c_str()) == NULL, "Duplicate pass name '%s'.", state->Name.c_str());
	
	// Parse state.
	rapidxml::xml_node<>* state_node = node->first_node("state");
	if (state_node != NULL)
	{
		state->State = Load_State(state_node);
	}
	
	// Parse pass type.
	std::string type_string = Get_Node_Value(node, "type", NULL);
	if (stricmp(type_string.c_str(), "mesh") == 0)
	{
		std::string mesh_name = Get_Node_Value(node, "mesh", NULL);

		state->Type = RenderPipeline_PassType::Mesh;
		state->Mesh = Get_Mesh_From_Name(mesh_name.c_str());

		DBG_ASSERT_STR(state->Mesh != NULL, "Unknown mesh '%s' in pass '%s'.", mesh_name.c_str(), state->Name.c_str());
	}
	else if (stricmp(type_string.c_str(), "scene") == 0)
	{		
		state->Type = RenderPipeline_PassType::Scene;
	}
	else if (stricmp(type_string.c_str(), "container") == 0)
	{		
		state->Type = RenderPipeline_PassType::Container;
	}
	else
	{
		DBG_ASSERT_STR(false, "Invalid type '%s' in pass '%s'.", type_string.c_str(), state->Name.c_str());
	}

	// Parse target.
	std::string target_name = Get_Node_Value(node, "target", "");
	if (target_name != "")
	{
		state->Target = Get_Target_From_Name(target_name.c_str());
		DBG_ASSERT_STR(state->Target != NULL, "Unknown target '%s' in pass '%s'.", target_name.c_str(), state->Name.c_str());	
	}
	
	// Parse shader.
	std::string shader_name = Get_Node_Value(node, "shader", "");
	if (shader_name != "")
	{
		state->Shader = Get_Shader_From_Name(shader_name.c_str());
		DBG_ASSERT_STR(state->Shader != NULL, "Unknown shader '%s' in pass '%s'.", shader_name.c_str(), state->Name.c_str());	
	}

	// Parse outputs.
	rapidxml::xml_node<>* outputs_node = node->first_node("outputs");
	if (outputs_node != NULL)
	{		
		rapidxml::xml_node<>* value_node = outputs_node->first_node("output");

		while (value_node != NULL)
		{
			const char* name = value_node->value();

			RenderPipeline_PassOutput* output = new RenderPipeline_PassOutput();

			if (stricmp(name, "BACK_BUFFER") == 0)
			{
				output->Type = RenderPipeline_PassOutputType::BackBuffer;
			}
			else
			{
				output->Type = RenderPipeline_PassOutputType::Texture;	
				output->Texture = Get_Texture_From_Name(name);

				DBG_ASSERT_STR(output->Texture != NULL, "Unknown texture '%s' in outputs for pass '%s'.", name, state->Name.c_str());
			}
			
			state->Outputs.push_back(output);

			value_node = value_node->next_sibling("output");
		}
	}

	// Parse for-each value.
	std::string foreach_name = Get_Node_Value(node, "foreach", "");
	if (foreach_name != "")
	{
		if (stricmp(foreach_name.c_str(), "light") == 0)
		{
			state->Foreach = RenderPipeline_PassForEachType::Light;
		}
		else if (stricmp(foreach_name.c_str(), "shadow_casting_light") == 0)
		{
			state->Foreach = RenderPipeline_PassForEachType::Shadow_Casting_Light;
		}
		else
		{
			DBG_ASSERT_STR(state->Shader != NULL, "Unknown foreach value '%s' in pass '%s'.", foreach_name.c_str(), state->Name.c_str());	
		}
	}

	// Load in passes.
	rapidxml::xml_node<>* passes = node->first_node("sub-passes", 0, false);
	if (passes != NULL)
	{
		rapidxml::xml_node<>* sub_pass = passes->first_node("pass", 0, false);
		while (sub_pass != NULL)
		{
			RenderPipeline_Pass* spass = Load_Pass(sub_pass);
			state->SubPasses.push_back(spass);
			
			sub_pass = sub_pass->next_sibling("pass", 0, false);
			DBG_LOG("Loaded pipeline sub-pass: %s", spass->Name.c_str());
		}
	}

	return state;
}
	
RenderPipeline_Slot* RenderPipeline::Load_Slot(rapidxml::xml_node<>* node)
{
	std::string enabled_string = Get_Attribute_Value(node, "enabled", "");

	RenderPipeline_Slot* state = new RenderPipeline_Slot();
	state->Name		= Get_Attribute_Value(node, "name", NULL);
	state->NameHash	= StringHelper::Hash(state->Name.c_str());
	
	DBG_ASSERT_STR(Get_Slot_From_Name(state->Name.c_str()) == NULL, "Duplicate slot name '%s'.", state->Name.c_str());
	
	// Parse sort type.
	std::string sort_string = Get_Node_Value(node, "sort", NULL);
	if (sort_string != "")
	{
		if (stricmp(sort_string.c_str(), "front_to_back") == 0)
		{
			state->SortType = RenderPipeline_SlotSortType::Front_To_Back;
		}
		else if (stricmp(sort_string.c_str(), "none") == 0)
		{		
			state->SortType = RenderPipeline_SlotSortType::None;
		}
		else
		{
			DBG_ASSERT_STR(false, "Invalid sort-type '%s' in slot '%s'.", sort_string.c_str(), state->Name.c_str());
		}
	}

	// Parse shader.
	std::string shader_name = Get_Node_Value(node, "shader", "");
	if (shader_name != "")
	{
		state->Shader = Get_Shader_From_Name(shader_name.c_str());
		DBG_ASSERT_STR(state->Shader != NULL, "Unknown shader '%s' in slot '%s'.", shader_name.c_str(), state->Name.c_str());	
	}

	// Parse passes
	std::string passes = Get_Node_Value(node, "passes", "");
	if (passes != "")
	{
		std::vector<std::string> split;
		if (StringHelper::Split(passes.c_str(), ',', split) > 0)
		{
			for (std::vector<std::string>::iterator passiter = split.begin(); passiter != split.end(); passiter++)
			{
				std::string pass_name = StringHelper::Trim((*passiter).c_str());
				RenderPipeline_Pass* pass = Get_Pass_From_Name(pass_name.c_str());
				DBG_ASSERT_STR(pass != NULL, "Unknown pass '%s' in slot '%s'.", pass_name.c_str(), state->Name.c_str());	

				state->Passes.push_back(pass);
			}
		}
	}

	return state;
}
	