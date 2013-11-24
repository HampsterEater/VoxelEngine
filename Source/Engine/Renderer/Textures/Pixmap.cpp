// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Textures\Pixmap.h"

int BPPForPixmapFormat[3] = 
{
	1,
	4,
	3
};

TextureFormat::Type TextureFormatForPixmapFormat[3] = 
{
	TextureFormat::Luminosity,
	TextureFormat::R8G8B8A8,
	TextureFormat::R8G8B8
};

Color PixmapWindow::Get_Pixel(int x, int y)
{
	int ox = Rect.X + x;
	int oy = Rect.Y + y;

	if (InvertX == true)
	{
		ox = Rect.X + (Rect.Width - (x + 1));
	}

	if (InvertY == true)
	{
		oy = Rect.Y + (Rect.Height - (y + 1));
	}

	return Pixmap->Get_Pixel(ox, oy);
}

Pixmap::Pixmap(int width, int height, PixmapFormat::Type format)
	: m_width(width)
	, m_height(height)
	, m_format(format)
{
	m_bpp = BPPForPixmapFormat[(int)format];
	m_texture_format = TextureFormatForPixmapFormat[(int)format];
	m_data = new unsigned char[width * height * m_bpp];
	m_pitch = m_width * m_bpp;
}

Pixmap::Pixmap(unsigned char* data, int width, int height, int pitch, PixmapFormat::Type format, bool owns_data)
	: m_width(width)
	, m_height(height)
	, m_pitch(pitch)
	, m_format(format)
	, m_data(data)
	, m_owns_data(owns_data)
{
	m_bpp = BPPForPixmapFormat[(int)format];
	m_texture_format = TextureFormatForPixmapFormat[(int)format];
}

Pixmap::~Pixmap()
{
	if (m_owns_data == true)
	{
		SAFE_DELETE(m_data);
	}
}

unsigned char* Pixmap::Get_Data()
{
	return m_data;
}

int Pixmap::Get_Width()
{
	return m_width;
}

int Pixmap::Get_Pitch()
{
	return m_pitch;
}

int Pixmap::Get_Height()
{
	return m_height;
}

PixmapFormat::Type Pixmap::Get_Format()
{
	return m_format;
}

TextureFormat::Type Pixmap::Get_Texture_Format()
{
	return m_texture_format;
}

void Pixmap::Clear(Color c)
{
	for (int x = 0; x < m_width; x++)
	{
		for (int y = 0; y < m_height; y++)
		{
			Set_Pixel(x, y, c);
		}
	}
}

void Pixmap::Set_Pixel(int x, int y, Color c)
{
	int offset = (m_pitch * (m_height - (y + 1))) + (x * m_bpp);
	if (m_bpp >= 1)
		m_data[offset] = c.R;
	if (m_bpp >= 2)
		m_data[offset + 1] = c.G;
	if (m_bpp >= 3)
		m_data[offset + 2] = c.B;
	if (m_bpp >= 4)
		m_data[offset + 3] = c.A;
}

Color Pixmap::Get_Pixel(int x, int y)
{
	Color result = Color(255, 255, 255, 255);
	int offset = (m_pitch * (m_height - (y + 1))) + (x * m_bpp);
	if (m_bpp >= 1)
		result.R = m_data[offset];
	if (m_bpp >= 2)
		result.G = m_data[offset + 1];
	if (m_bpp >= 3)
		result.B = m_data[offset + 2];
	if (m_bpp >= 4)
		result.A = m_data[offset + 3];
	return result;
}

PixmapWindow Pixmap::Window(Rect rect, bool invert_x, bool invert_y)
{
	PixmapWindow window;
	window.Pixmap = this;
	window.Rect = rect;
	window.InvertX = invert_x;
	window.InvertY = invert_y;
	return window;
}

void Pixmap::Paste(Point position, PixmapWindow rect)
{
	DBG_ASSERT(position.X >= 0 && position.X + rect.Rect.Width < m_width);
	DBG_ASSERT(position.Y >= 0 && position.Y + rect.Rect.Height < m_height);

	for (int x = 0; x < rect.Rect.Width; x++)
	{
		for (int y = 0; y < rect.Rect.Height; y++)
		{
			Color source = rect.Get_Pixel(x, y);
			Set_Pixel(position.X + x, position.Y + y, source);
		}
	}
}