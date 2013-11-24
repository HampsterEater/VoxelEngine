// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Atlases\AtlasRenderer.h"
#include "Engine\Renderer\Atlases\AtlasHandle.h"
#include "Engine\Renderer\Atlases\Atlas.h"

#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\Renderer\Textures\Pixmap.h"

Atlas::Atlas(ConfigFile* config)
	: m_textures_locked(false)
	, m_current_dirty_texture_index(-1)
{
	// Load configuration.
	m_texture_size  = config->Get<int>("settings/texture-size");
	m_max_textures	= config->Get<int>("settings/max-textures");
	
	// Create texture buffer.
	m_textures = new AtlasTexture*[m_max_textures];
	memset(m_textures, 0, m_max_textures * sizeof(Texture*));	
}

Atlas::Atlas()
	: m_textures_locked(false)
	, m_current_dirty_texture_index(-1)
{
}

Atlas::~Atlas()
{
	for (HashTable<AtlasFrame*, unsigned int>::Iterator iter = m_frames.Begin(); iter != m_frames.End(); iter++)
	{
		delete *iter;
	}

	for (HashTable<AtlasAnimation*, unsigned int>::Iterator iter = m_animations.Begin(); iter != m_animations.End(); iter++)
	{
		delete *iter;
	}

	for (int i = 0; i < m_max_textures; i++)
	{
		AtlasTexture* texture = m_textures[i];
		if (texture != NULL)
		{
			SAFE_DELETE(texture);
		}
	}

	SAFE_DELETE(m_textures);

	m_frames.Clear();
	m_frames_list.Clear();
	m_animations.Clear();
}

void Atlas::Lock_Textures()
{
	m_textures_locked = true;
}

AtlasTexture** Atlas::Get_Textures(int& texture_count)
{
	texture_count = m_current_dirty_texture_index + 1;
	return m_textures;
}

void Atlas::Unlock_Textures()
{
	m_textures_locked = false;

	for (int i = 0; i < m_max_textures; i++)
	{
		AtlasTexture* texture = m_textures[i];
		if (texture != NULL && texture->Is_Dirty == true)
		{
			texture->Texture->Set_Pixmap(texture->Pixmap);
			texture->Is_Dirty = false;
		}
	}
 }

HashTable<AtlasFrame*, unsigned int>& Atlas::Get_Frames()
{
	return m_frames;
}

LinkedList<AtlasFrame*>& Atlas::Get_Frames_List()
{
	return m_frames_list;
}

HashTable<AtlasAnimation*, unsigned int>& Atlas::Get_Animations()
{
	return m_animations;
}

AtlasFrame* Atlas::Get_Frame(const char* name)
{
	return m_frames.Get(StringHelper::Hash(name));
}

AtlasFrame* Atlas::Get_Frame(unsigned int name_hash)
{
	return m_frames.Get(name_hash);
}

void Atlas::Add_Frame(const char* name, Texture* source_texture, Rect texture_window, Point origin)
{
	Renderer*			renderer		= Renderer::Get();
	bool				new_texture		= false;
	Rect				output_rect;
	int					source_width	= source_texture->Get_Width();
	int					source_height	= source_texture->Get_Height();
	unsigned char*		source_data		= reinterpret_cast<unsigned char*>(source_texture->Get_Pixmap()->Get_Data());
	TextureFormat::Type source_format	= source_texture->Get_Format();
	Pixmap*				source_pixmap	= source_texture->Get_Pixmap();

	// Origins that are fractional are considered a fraction
	// of the source texture size (0.5 = 50% etc)
	if (origin.X < 1)
	{
		origin.X = source_width * origin.X;
	}
	if (origin.Y < 1)
	{
		origin.Y = source_height * origin.Y;
	}

	if (m_current_dirty_texture_index < 0)
	{
		new_texture = true;
	}
	else
	{
		new_texture = !m_textures[m_current_dirty_texture_index]->Packer.Pack(Point(texture_window.Width, texture_window.Height), output_rect);
	}

	if (new_texture == true)
	{	
		DBG_ASSERT_STR(++m_current_dirty_texture_index < m_max_textures, "Ran out of atlas texture space.");

		m_textures[m_current_dirty_texture_index]			= new AtlasTexture();
		m_textures[m_current_dirty_texture_index]->Packer	= RectanglePacker(m_texture_size, m_texture_size);
		m_textures[m_current_dirty_texture_index]->Pixmap	= new Pixmap(m_texture_size, m_texture_size, PixmapFormat::R8G8B8A8);
		m_textures[m_current_dirty_texture_index]->Pixmap->Clear(Color(0, 0, 0, 0));
		m_textures[m_current_dirty_texture_index]->Texture = renderer->Create_Texture(m_textures[m_current_dirty_texture_index]->Pixmap, TextureFlags::None);
	
		bool packed = m_textures[m_current_dirty_texture_index]->Packer.Pack(Point(texture_window.Width, texture_window.Height), output_rect);
		DBG_ASSERT_STR(packed, "Could not pack frame into brand new texture! To large?");
	}

	// Work out glyph texture position etc.
	int pixel_x	= output_rect.X;
	int pixel_y	= output_rect.Y;
	int pixel_w	= output_rect.Width;
	int pixel_h	= output_rect.Height;		
	float uv_w	= (output_rect.Width) / float(m_texture_size);		
	float uv_h	= (output_rect.Height) / float(m_texture_size);
	float uv_x	= (output_rect.X) / float(m_texture_size);
	float uv_y	= (output_rect.Y) / float(m_texture_size);

	// Only render if we have a bitmap for this glyph.
	AtlasTexture* texture = m_textures[m_current_dirty_texture_index];

	// Copy pixels ot output.
	PixmapWindow window = source_pixmap->Window(Rect(texture_window.X, texture_window.Y, output_rect.Width, output_rect.Height));
	texture->Pixmap->Paste(Point(output_rect.X, output_rect.Y), window);

	// Add glyph to list.	
	AtlasFrame* frame		= new AtlasFrame();
	frame->Name				= name;
	frame->NameHash			= StringHelper::Hash(name);
	frame->Texture			= m_textures[m_current_dirty_texture_index];
	frame->TextureIndex		= m_current_dirty_texture_index;
	frame->UV				= Rect(uv_x, uv_y, uv_w, uv_h);
	frame->Rectangle		= Rect(pixel_x, pixel_y, pixel_w, pixel_h);
	frame->Origin			= origin;

	m_textures[m_current_dirty_texture_index]->Is_Dirty = true;

	m_frames.Set(frame->NameHash, frame);
	m_frames_list.Add(frame);
}

void Atlas::Add_Animation(const char* name, float speed, AtlasAnimationMode::Type mode, std::vector<AtlasFrame*> frames)
{
	AtlasAnimation* anim	= new AtlasAnimation();
	anim->Name				= name;
	anim->NameHash			= StringHelper::Hash(name);
	anim->Mode				= mode;
	anim->Speed				= speed;
	anim->Frames			= frames;

	m_animations.Set(anim->NameHash, anim);
}

bool Atlas::Load_Compiled_Config(ConfigFile* config)
{	
	// Load configuration.
	m_texture_size  = config->Get<int>("settings/texture-size");
	m_max_textures	= config->Get<int>("settings/max-textures");
	
	// Create texture buffer.
	m_textures = new AtlasTexture*[m_max_textures];
	memset(m_textures, 0, m_max_textures * sizeof(Texture*));	

	// Load in all textures.
	std::vector<const char*> textures = config->Get<std::vector<const char*>>("textures/texture");
	DBG_ASSERT_STR(textures.size() <= m_max_textures, "Compiled atlas contains more textures that max-textures.");

	for (std::vector<const char*>::iterator iter = textures.begin(); iter != textures.end(); iter++)
	{
		m_current_dirty_texture_index++;

		m_textures[m_current_dirty_texture_index]			= new AtlasTexture();
		m_textures[m_current_dirty_texture_index]->Is_Dirty = false;
		m_textures[m_current_dirty_texture_index]->Texture	= TextureFactory::Load_Without_Handle(*iter, TextureFlags::None);

		if (m_textures[m_current_dirty_texture_index]->Texture == NULL)
		{
			DBG_LOG("Failed to load compiled texture '%s'.", *iter);
			return false;
		}

		m_textures[m_current_dirty_texture_index]->Pixmap	= m_textures[m_current_dirty_texture_index]->Texture->Get_Pixmap();
		m_textures[m_current_dirty_texture_index]->Packer   = RectanglePacker(0, 0);		
	}

	// We don't want to try appending glyphs to last loaded texture, so move to the "next" one.
	m_current_dirty_texture_index++;

	// Load all frames.
	std::vector<ConfigFileNode> frames = config->Get<std::vector<ConfigFileNode>>("frames/frame");
	for (std::vector<ConfigFileNode>::iterator iter = frames.begin(); iter != frames.end(); iter++)
	{
		ConfigFileNode node = *iter;

		int texture_index = config->Get<int>("texture", node, true);
		DBG_ASSERT_STR(texture_index < m_max_textures, "Glyph attempted to reference out of bounds texture index '%i'.", texture_index);

		// Add frame to list.	
		AtlasFrame* frame		= new AtlasFrame();	
		frame->Name				= config->Get<const char*>("name", node, true);
		frame->NameHash			= config->Get<unsigned int>("namehash", node, true);
		frame->Rectangle		= config->Get<Rect>("rectangle", node, true);
		frame->Texture			= m_textures[texture_index];	
		frame->TextureIndex		= texture_index;
		frame->UV				= config->Get<Rect>("uv", node, true);
		frame->Origin			= config->Get<Point>("origin", node, true);
		
		m_frames.Set(frame->NameHash, frame);
		m_frames_list.Add(frame);
	}

	// Load all animations.
	std::vector<ConfigFileNode> anims = config->Get<std::vector<ConfigFileNode>>("animations/animation");
	for (std::vector<ConfigFileNode>::iterator iter = anims.begin(); iter != anims.end(); iter++)
	{
		ConfigFileNode node = *iter;

		// Add animation to list.	
		AtlasAnimation* anim	= new AtlasAnimation();	
		anim->Name				= config->Get<const char*>("name", node, true);
		anim->NameHash			= config->Get<unsigned int>("namehash", node, true);
		anim->Speed				= config->Get<float>("speed", node, true);

		const char* anim_name = config->Get<const char*>("mode", node, true);
		if (stricmp(anim_name, "once") == 0)
		{
			anim->Mode = AtlasAnimationMode::Once;
		}
		else if (stricmp(anim_name, "loop") == 0)
		{
			anim->Mode = AtlasAnimationMode::Loop;
		}
		else
		{
			DBG_ASSERT_STR(false, "Invalid animation mode '%s' in compiled file.", anim_name);
		}

		// Load frames.
		std::vector<ConfigFileNode> frames = config->Get<std::vector<ConfigFileNode>>("frame", node);
		for (std::vector<ConfigFileNode>::iterator subiter = frames.begin(); subiter != frames.end(); subiter++)
		{
			ConfigFileNode subnode = *subiter;

			AtlasFrame* frame = Get_Frame(config->Get<unsigned int>("namehash", subnode, true));
			DBG_ASSERT(frame != NULL, "Compiled file contains invalid frame in animation '%s'.", anim->Name.c_str());

			anim->Frames.push_back(frame);
		}

		m_animations.Set(anim->NameHash, anim);
	}

	return true;
}