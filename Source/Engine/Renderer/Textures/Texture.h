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
		R8G8B8,
		R32FG32FB32FA32F,
		DepthFormat,
		StencilFormat,
		Luminosity
	};
};

struct TextureFlags
{
	enum Type
	{
		None = 0,
		AllowRepeat = 1,
		LinearFilter = 2
	};
};

class Texture
{
protected:
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
	int						Get_Width	 () const;
	int						Get_Height	 () const;
	int						Get_Pitch	 () const;
	char*					Get_Data	 () const;
	TextureFormat::Type		Get_Format	 () const;
	
	// Note: When setting data ownership of the buffer is transfered to the texture 
	//		 object which will deal with deallocation.
	virtual void			Set_Data	 (char* buffer) = 0;

};

#endif

