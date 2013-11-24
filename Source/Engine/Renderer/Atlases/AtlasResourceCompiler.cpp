// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Atlases\AtlasFactory.h"
#include "Engine\Renderer\Atlases\Atlas.h"
#include "Engine\Renderer\Atlases\AtlasResourceCompiler.h"
#include "Engine\IO\StreamFactory.h"
#include "Generic\Types\HashTable.h"
#include "Generic\Helper\StringHelper.h"

#include "Engine\Renderer\Textures\Texture.h"
#include "Engine\Renderer\Textures\TextureFactory.h"

AtlasResourceCompiler::AtlasResourceCompiler(const char* url)
	: m_config_file(NULL)
{
	Platform*	platform	= Platform::Get();

	m_input_path			= url;
	m_input_directory		= platform->Extract_Directory(url);
	m_input_filename		= platform->Extract_Filename(url);
	m_output_directory		= Directory_To_Output_Directory(m_input_directory);
	m_output_path			= platform->Join_Path(m_output_directory, m_input_filename);
}

AtlasResourceCompiler::~AtlasResourceCompiler()
{
	SAFE_DELETE(m_config_file);
}

bool AtlasResourceCompiler::Should_Compile()
{
	Platform* platform = Platform::Get();

	// If input dosen't exist, the wtf.
	if (!platform->Is_File(m_input_path.c_str()))
	{
		DBG_ASSERT_STR(false, "Attempt to compile non-existing resource '%s'.", m_input_path.c_str());
		return false;
	}

	// If input has been modified, compile is definitely required.
	if (Check_File_Timestamp(m_input_path))
	{
		return true;
	}

	// Read in the XML, so we can determine if there are any other dependent files.
	if (!Load_Config())
	{
		return false;
	}

	// Check dependent files.
	for (std::vector<std::string>::iterator iter = m_dependent_files.begin(); iter != m_dependent_files.end(); iter++)
	{
		std::string& path = *iter;
		if (Check_File_Timestamp(path.c_str()))
		{
			return true;
		}
	}

	return false;
}

std::string AtlasResourceCompiler::Get_Compiled_Path()
{
	return m_output_path;
}

bool AtlasResourceCompiler::Load_Config()
{
	Platform* platform = Platform::Get();

	// Destroy old config file.
	SAFE_DELETE(m_config_file);
	
	// Load XML file.
	m_config_file = new ConfigFile();
	if (!m_config_file->Load(m_input_path.c_str()))
	{
		DBG_ASSERT_STR(false, "Attempt to compile invalid config file '%s'.", m_input_path.c_str());
		return false;
	}

	// Load dependent files.
	std::vector<ConfigFileNode> images = m_config_file->Get<std::vector<ConfigFileNode>>("images/image");
	for (std::vector<ConfigFileNode>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		ConfigFileNode node = *iter;		

		const char* type = m_config_file->Get<const char*>("type", node, true);
		const char* file = m_config_file->Get<const char*>("file", node, true);

		if (stricmp(type, "single") == 0)
		{
			m_dependent_files.push_back(file);
		}		
		else if (stricmp(type, "multiple") == 0)
		{
			Expand_Multiple_Path(file, m_dependent_files);
		}
		else if (stricmp(type, "atlas") == 0)
		{
			m_dependent_files.push_back(file);
		}
		else if (stricmp(type, "grid") == 0)
		{
			m_dependent_files.push_back(file);
		}
		else
		{
			DBG_ASSERT_STR(false, "Unknown or invalid source-image type '%s'", type);
		}
	}

	return true;
}

void AtlasResourceCompiler::Expand_Multiple_Path(const char* path, std::vector<std::string>& result)
{
	std::string file_path = path;

	int hash_mark = file_path.find('#');
	DBG_ASSERT_STR(hash_mark != std::string::npos, "Image marked as type 'multiple' does not contain replacement hash mark.");

	std::string split_left  = file_path.substr(0, hash_mark);
	std::string split_right = file_path.substr(hash_mark + 1);

	int counter = 0;
	while (true)
	{
		std::string dep_path = split_left + StringHelper::To_String(counter++) + split_right;

		if (Platform::Get()->Is_File(dep_path.c_str()))
		{
			result.push_back(dep_path);
		}
		else
		{
			break;
		}
	}
}

bool AtlasResourceCompiler::Compile()
{
	Platform* platform = Platform::Get();

	// Load configuration if we haven't already.
	if (m_config_file == NULL)
	{
		if (!Load_Config())
		{
			return false;
		}
	}

	// Compile time.
	DBG_LOG("Compiling atlas resource '%s'.", m_input_path.c_str());

	// Create new atlas, and start adding rectangles.
	Atlas* atlas = new Atlas(m_config_file);
	atlas->Lock_Textures();

	// Add each of the source-images.
	std::vector<ConfigFileNode> images = m_config_file->Get<std::vector<ConfigFileNode>>("images/image");
	for (std::vector<ConfigFileNode>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		ConfigFileNode node = *iter;		

		const char* type = m_config_file->Get<const char*>("type", node, true);
		const char* file = m_config_file->Get<const char*>("file", node, true);

		Point origin = Point(0, 0);

		if (m_config_file->Contains("origin", node, true))
		{
			origin = m_config_file->Get<Point>("origin", node, true);
		}

		if (stricmp(type, "single") == 0)
		{
			const char* name = m_config_file->Get<const char*>("name", node, true);

			Texture* texture = TextureFactory::Load_Without_Handle(file, TextureFlags::None);
			DBG_ASSERT_STR(texture != NULL, "Could not load dependent texture '%s'.", file);
			
			atlas->Add_Frame(name, texture, Rect(0, 0, texture->Get_Width(), texture->Get_Height()), origin);
		}		
		else if (stricmp(type, "multiple") == 0)
		{
			const char* name = m_config_file->Get<const char*>("name", node, true);

			// Grab all the file paths.
			std::string name_str = name;
			std::vector<std::string> files;
			Expand_Multiple_Path(file, files);

			// Grab hash mark.
			int hash_mark = name_str.find('#');
			DBG_ASSERT_STR(hash_mark != std::string::npos, "Image marked as type 'multiple' does not contain replacement hash mark.");
			std::string split_left  = name_str.substr(0, hash_mark);
			std::string split_right = name_str.substr(hash_mark + 1);

			// Add each file.
			int counter = 0;
			for (std::vector<std::string>::iterator subiter = files.begin(); subiter != files.end(); subiter++)
			{
				std::string sub_path = *subiter;
				std::string sub_name = split_left + StringHelper::To_String(counter++) + split_right;
				
				Texture* texture = TextureFactory::Load_Without_Handle(sub_path.c_str(), TextureFlags::None);
				DBG_ASSERT_STR(texture != NULL, "Could not load dependent texture '%s'.", sub_path);

				atlas->Add_Frame(sub_name.c_str(), texture, Rect(0, 0, texture->Get_Width(), texture->Get_Height()), origin);
			}
		}
		else if (stricmp(type, "grid") == 0)
		{
			const char* name = m_config_file->Get<const char*>("name", node, true);

			Texture* texture = TextureFactory::Load_Without_Handle(file, TextureFlags::None);
			DBG_ASSERT_STR(texture != NULL, "Could not load dependent texture '%s'.", file);
		
			// Grab hash mark.
			std::string name_str = name;
			int hash_mark = name_str.find('#');
			DBG_ASSERT_STR(hash_mark != std::string::npos, "Image marked as type 'grid' does not contain replacement hash mark.");
			std::string split_left  = name_str.substr(0, hash_mark);
			std::string split_right = name_str.substr(hash_mark + 1);

			// Iterate over each sub-image.			
			int   frame_width		= m_config_file->Get<int>("width", node, true);
			int   frame_height		= m_config_file->Get<int>("height", node, true);
			int   frame_hspace		= m_config_file->Get<int>("hspace", node, true);
			int   frame_vspace		= m_config_file->Get<int>("vspace", node, true);			
			float texture_width		= (float)texture->Get_Width();
			float texture_height	= (float)texture->Get_Height();
			int	  cell_count_u		= texture_width  / (frame_width + frame_hspace);
			int	  cell_count_v		= texture_height / (frame_height + frame_vspace);
			int	  cell_count		= cell_count_u * cell_count_v;

			for (int i = 0; i < cell_count; i++)
			{
				int u = i % cell_count_u;
				int v = i / cell_count_u;

				int u_offset = floor(u * (frame_width + frame_hspace));
				int v_offset = floor(v * (frame_height + frame_vspace));
				
				std::string sub_name = split_left + StringHelper::To_String(i) + split_right;
				atlas->Add_Frame(sub_name.c_str(), texture, Rect(u_offset, v_offset, frame_width, frame_height), origin);
			}
		}
		else if (stricmp(type, "atlas") == 0)
		{
			Texture* texture = TextureFactory::Load_Without_Handle(file, TextureFlags::None);
			DBG_ASSERT_STR(texture != NULL, "Could not load dependent texture '%s'.", file);

			// Calculate frame positions for atlas.
			int frame_width			= m_config_file->Get<int>("width", node, true);
			int frame_height		= m_config_file->Get<int>("height", node, true);
			int frame_hspace		= m_config_file->Get<int>("hspace", node, true);
			int frame_vspace		= m_config_file->Get<int>("vspace", node, true);	
			float texture_width		= (float)texture->Get_Width();
			float texture_height	= (float)texture->Get_Height();
			int	  cell_count_u		= texture_width  / (frame_width + frame_hspace);
			int	  cell_count_v		= texture_height / (frame_height + frame_vspace);
			int	  cell_count		= cell_count_u * cell_count_v;

			// Iterate over each sub-image.	
			std::vector<ConfigFileNode> sub_images = m_config_file->Get<std::vector<ConfigFileNode>>("subimage", node);
			for (std::vector<ConfigFileNode>::iterator subiter = sub_images.begin(); subiter != sub_images.end(); subiter++)
			{
				ConfigFileNode sub_node = *subiter;	
				
				const char* sub_type = m_config_file->Get<const char*>("type", sub_node, true);
				const char* sub_name = m_config_file->Get<const char*>("name", sub_node, true);
				
				if (stricmp(sub_type, "single") == 0)
				{
					int frame		= m_config_file->Get<int>("frame", sub_node, true);
					int u			= frame % cell_count_u;
					int v			= frame / cell_count_u;
					int u_offset	= floor(u * (frame_width + frame_hspace));
					int v_offset	= floor(v * (frame_height + frame_vspace));
						
					atlas->Add_Frame(sub_name, texture, Rect(u_offset, v_offset, frame_width, frame_height), origin);
				}
				else if (stricmp(sub_type, "strip") == 0)
				{
					int to_frame   = m_config_file->Get<int>("to_frame", sub_node, true);
					int from_frame = m_config_file->Get<int>("from_frame", sub_node, true);
					DBG_ASSERT_STR(to_frame >= from_frame, "Invalid strip range %i to %i.", from_frame, to_frame);
					
					std::string sub_name_str	= sub_name;
					int			hash_mark		= sub_name_str.find('#');
					DBG_ASSERT_STR(hash_mark != std::string::npos, "Sub-image marked as type 'strip' does not contain replacement hash mark.");

					std::string split_left  = sub_name_str.substr(0, hash_mark);
					std::string split_right = sub_name_str.substr(hash_mark + 1);

					for(int frame = from_frame; frame <= to_frame; frame++)
					{
						int u			= frame % cell_count_u;
						int v			= frame / cell_count_u;
						int u_offset	= floor(u * (frame_width + frame_hspace));
						int v_offset	= floor(v * (frame_height + frame_vspace));
						
						std::string sub_sub_name = split_left + StringHelper::To_String(frame) + split_right;
						atlas->Add_Frame(sub_sub_name.c_str(), texture, Rect(u_offset, v_offset, frame_width, frame_height), origin);
					}
				}
				else
				{
					DBG_ASSERT_STR(false, "Unknown or invalid sub-image type '%s'", sub_type);
				}
			}
		}
		else
		{
			DBG_ASSERT_STR(false, "Unknown or invalid source-image type '%s'", type);
		}
	}

	// Add each of the animations.
	std::vector<ConfigFileNode> animations = m_config_file->Get<std::vector<ConfigFileNode>>("animations/animation");
	for (std::vector<ConfigFileNode>::iterator iter = animations.begin(); iter != animations.end(); iter++)
	{
		ConfigFileNode node = *iter;		
		
		const char* name = m_config_file->Get<const char*>("name", node, true);
		const char* mode = m_config_file->Get<const char*>("mode", node, true);
		float	   speed = m_config_file->Get<float>("speed", node, true);
		std::vector<AtlasFrame*> out_frames;
		
		// Decode the animation mode.
		AtlasAnimationMode::Type anim_mode;
		if (stricmp(mode, "once") == 0)
		{
			anim_mode = AtlasAnimationMode::Once;
		}
		else if (stricmp(mode, "loop") == 0)
		{
			anim_mode = AtlasAnimationMode::Loop;
		}
		else
		{
			DBG_ASSERT_STR(false, "Unknown or invalid animation mode '%s'", mode);
		}
		
		// Work out the names of each of the frames.
		std::vector<ConfigFileNode> frames = m_config_file->Get<std::vector<ConfigFileNode>>("frame", node);
		for (std::vector<ConfigFileNode>::iterator frameiter = frames.begin(); frameiter != frames.end(); frameiter++)
		{
			ConfigFileNode frame_node = *frameiter;			
			const char*	   frame_type = m_config_file->Get<const char*>("type", frame_node, true);
			const char*	   frame_name = m_config_file->Get<const char*>("name", frame_node, true);

			if (stricmp(frame_type, "single") == 0)
			{
				AtlasFrame* frame = atlas->Get_Frame(frame_name);
				DBG_ASSERT(frame != NULL, "Invalid frame name '%s' in animation '%s'.", frame_name, name);
				out_frames.push_back(frame);
			}
			else if (stricmp(frame_type, "multiple") == 0)
			{
				LinkedList<AtlasFrame*>& frames = atlas->Get_Frames_List();
				
				std::string name_str = frame_name;

				int hash_mark = name_str.find('#');
				DBG_ASSERT_STR(hash_mark != std::string::npos, "Animation marked as type 'multiple' does not contain replacement hash mark.");

				std::string split_left  = name_str.substr(0, hash_mark);
				std::string split_right = name_str.substr(hash_mark + 1);
				
				for (LinkedList<AtlasFrame*>::Iterator animiter = frames.Begin(); animiter != frames.End(); animiter++)
				{
					AtlasFrame* anim = *animiter;

					if (anim->Name.size() > split_left.size() + split_right.size())
					{
						if (anim->Name.substr(0, split_left.size()) == split_left &&
							anim->Name.substr((anim->Name.size() - split_right.size()), split_right.size()) == split_right)
						{
							AtlasFrame* frame = atlas->Get_Frame(anim->Name.c_str());
							DBG_ASSERT(frame != NULL, "Invalid frame name '%s' in animation '%s'.", anim->Name, name);
							out_frames.push_back(frame);
						}
					}
				}
			}
			else
			{
				DBG_ASSERT_STR(false, "Unknown or invalid frame type '%s'", frame_type);
			}
		}

		// Add the animation.
		atlas->Add_Animation(name, speed, anim_mode, out_frames);
	}

	// Save compiled file.
	atlas->Unlock_Textures();

	// Configuration settings.
	ConfigFile config;
	
	// Store configuration settings.
	config.Set<const char*>("settings/texture-size",		m_config_file->Get<const char*>("settings/texture-size"));
	config.Set<const char*>("settings/max-textures",		m_config_file->Get<const char*>("settings/max-textures"));

	// Save pngs.
	int texture_count = 0;
	AtlasTexture** textures = atlas->Get_Textures(texture_count);

	for (int i = 0; i < texture_count; i++)
	{
		AtlasTexture* texture = textures[i];		

		std::string output_path = (m_output_path + "." + StringHelper::To_String(i) + ".png");

		// Save texture file.
		TextureFactory::Save(output_path.c_str(), texture->Texture, TextureFlags::None);

		// Store texture spec in xml.
		ConfigFileNode node = config.New_Node("textures/texture");
		config.Set(NULL, output_path.c_str(), node, false);
	}

	// Save frame configuration.
	config.New_Node("frames");
	HashTable<AtlasFrame*, unsigned int>& frames = atlas->Get_Frames();
	for (HashTable<AtlasFrame*, unsigned int>::Iterator iter = frames.Begin(); iter != frames.End(); iter++)
	{
		AtlasFrame* frame = iter.Get_Value();
		
		// Store character spec in xml.
		ConfigFileNode node = config.New_Node("frames/frame");
		config.Set("name",			frame->Name.c_str(),				node, true);
		config.Set("namehash",		frame->NameHash,					node, true);
		config.Set("texture",		frame->TextureIndex,				node, true);
		config.Set("rectangle",		frame->Rectangle,					node, true);
		config.Set("uv",			frame->UV,							node, true);
		config.Set("origin",		frame->Origin,						node, true);
	}

	// Save animation configuration.
	config.New_Node("animations");
	HashTable<AtlasAnimation*, unsigned int>& anims = atlas->Get_Animations();
	for (HashTable<AtlasAnimation*, unsigned int>::Iterator iter = anims.Begin(); iter != anims.End(); iter++)
	{
		AtlasAnimation* frame = iter.Get_Value();
		
		// Store character spec in xml.
		ConfigFileNode node = config.New_Node("animations/animation");
		config.Set("name",			frame->Name.c_str(),				node, true);
		config.Set("namehash",		frame->NameHash,					node, true);
		switch (frame->Mode)
		{
			case AtlasAnimationMode::Loop: config.Set("mode", "loop", node, true); break;
			case AtlasAnimationMode::Once: config.Set("mode", "once", node, true); break;
			default: DBG_ASSERT_STR(false, "Invalid animation mode.");
		}
		config.Set("speed",			frame->Speed,						node, true);

		for (std::vector<AtlasFrame*>::iterator iter = frame->Frames.begin(); iter != frame->Frames.end(); iter++)
		{
			AtlasFrame* frame = (*iter);
			ConfigFileNode subnode = config.New_Node("frame", node);

			config.Set("name", frame->Name.c_str(), subnode, true);
			config.Set("namehash", frame->NameHash, subnode, true);
		}
	}

	// Save font xml configuration.
	config.Save(m_output_path.c_str());

	// Update timestamps.
	Update_File_Timestamp(m_input_path);

	// Update timestamps on dependent files.
	for (std::vector<std::string>::iterator iter = m_dependent_files.begin(); iter != m_dependent_files.end(); iter++)
	{
		std::string path = (*iter);
		DBG_ASSERT_STR(Platform::Get()->Is_File(path.c_str()), "Could not find dependent file '%s'.", path.c_str());

		Update_File_Timestamp(path.c_str());
	}

	DBG_LOG("Finished compiling to '%s'.", m_output_path.c_str());

	return true;
}

Atlas* AtlasResourceCompiler::Load_Compiled()
{
	std::string compiled_path = Get_Compiled_Path();
	Platform* platform = Platform::Get();

	// Compile time.
	DBG_LOG("Loading atlas resource '%s'.", compiled_path.c_str());
	
	// Load configuration settings.
	ConfigFile config;
	if (!config.Load(compiled_path.c_str()))
	{
		DBG_LOG("Failed to load atlas, config file could not be found: '%s'", compiled_path.c_str());
		return NULL;
	}

	// Load the compile atlas file.
	Atlas* atlas = new Atlas();
	if (!atlas->Load_Compiled_Config(&config))
	{
		return NULL;
	}

	DBG_LOG("Finished loading compiled atlas from '%s'.", compiled_path.c_str());
	return atlas;
}