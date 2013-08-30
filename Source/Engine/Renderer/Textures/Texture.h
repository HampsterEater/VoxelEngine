// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXTURES_TEXTURE_
#define _ENGINE_RENDERER_TEXTURES_TEXTURE_

struct TextureFormat
{
	enum Type
	{
		R8G8B8A8,
		R8G8B8
	};
};

class Texture
{
private:
	char*				m_data;
	int					m_width;
	int					m_height;
	int					m_pitch;
	TextureFormat::Type m_format;
	
protected:

	// Constructor!
	Texture(char* data, int width, int height, int pitch, TextureFormat::Type format);

public:

	// Destructor!
	virtual ~Texture();

	// Members that have to be overidden.
	int						Get_Width	 ();
	int						Get_Height	 ();
	int						Get_Pitch	 ();
	const char*				Get_Data	 ();
	TextureFormat::Type		Get_Format	 ();

};

#endif

