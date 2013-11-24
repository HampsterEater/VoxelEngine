// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_ATLASES_ATLAS_
#define _ENGINE_RENDERER_ATLASES_ATLAS_

#include "Generic\Types\HashTable.h"
#include "Generic\Types\LinkedList.h"
#include "Generic\Types\Rectangle.h"
#include "Generic\Types\Point.h"
#include "Generic\Math\RectanglePacker.h"

#include "Engine\Config\ConfigFile.h"

class Texture;
class Pixmap;

struct AtlasAnimationMode
{
	enum Type
	{
		Once,
		Loop
	};
};

struct AtlasTexture
{
public:
	Texture*		Texture;
	Pixmap*			Pixmap;
	bool			Is_Dirty;
	RectanglePacker	Packer;
};

struct AtlasFrame
{
	std::string		Name;
	unsigned int	NameHash;
	Rect			UV;
	Rect			Rectangle;
	AtlasTexture*	Texture;
	int				TextureIndex;
	Point			Origin;
};

struct AtlasAnimation
{
	std::string					Name;
	unsigned int				NameHash;
	AtlasAnimationMode::Type	Mode;
	float						Speed;
	std::vector<AtlasFrame*>	Frames;
};

class Atlas
{
private:
	int m_texture_size;
	int m_max_textures;
	
	AtlasTexture**								m_textures;
	HashTable<AtlasFrame*, unsigned int>		m_frames;
	LinkedList<AtlasFrame*>						m_frames_list;
	HashTable<AtlasAnimation*, unsigned int>	m_animations;
	int											m_current_dirty_texture_index;
	bool										m_textures_locked;

protected:
	friend class AtlasResourceCompiler;

	// Methods to construct atlases.
	void			Lock_Textures();
	AtlasTexture**	Get_Textures(int& texture_count);
	void			Unlock_Textures();

	HashTable<AtlasFrame*, unsigned int>&	  Get_Frames		();
	LinkedList<AtlasFrame*>&				  Get_Frames_List	();
	HashTable<AtlasAnimation*, unsigned int>& Get_Animations	();

	void Add_Frame	  (const char* name, Texture* texture, Rect texture_window, Point origin);
	void Add_Animation(const char* name, float speed, AtlasAnimationMode::Type mode, std::vector<AtlasFrame*> frames);

	bool Load_Compiled_Config(ConfigFile* config);

public:

	// Destructor!
	Atlas();
	Atlas(ConfigFile* config);
	~Atlas();

	// Retrieving frames.
	AtlasFrame* Get_Frame(const char* name);
	AtlasFrame* Get_Frame(unsigned int name_hash);

};

#endif

