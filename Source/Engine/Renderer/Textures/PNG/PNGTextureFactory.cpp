// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Textures\Texture.h"
#include "Engine\Renderer\Textures\PNG\PNGTextureFactory.h"

#include "Engine\Renderer\Renderer.h"

#include "Engine\IO\StreamFactory.h"

#include <string>

void PNGTextureFactory::libpng_read_function(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_voidp ptr = png_get_io_ptr(png_ptr);
	((Stream*)ptr)->Read((char*)data, 0, length);
}

Texture* PNGTextureFactory::Try_Load(const char* url)
{
	// Can we open this path as a file?
	Stream* stream = StreamFactory::Open(url, StreamMode::Read);
	if (stream == NULL)
	{
		return NULL;
	}

	// Big enough to be a PNG?
	if (stream->Length() < 8)
	{
		delete stream;
		return NULL; 
	}

	// Read in header.
	png_byte header[8];
	stream->Read((char*)header, 0, 8);

	// Is header valid for a PNG?
	if (png_sig_cmp(header, 0, 8))
	{
		delete stream;
		return NULL;
	}

	// Create initial structures required for reading.
	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
	{
		delete stream;
		return NULL;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);

		delete stream;
		return NULL;
    }
	
	png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info)
    {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);

		delete stream;
		return NULL;
    }

	// Setup error handler.
	if (setjmp(png_jmpbuf(png_ptr)))
	{
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

		delete stream;
		return NULL;
	}

	// Setup custom reading.
	png_set_read_fn(png_ptr, (png_voidp)stream, libpng_read_function);
	
	// init png reading
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

	// Get general information about image.
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type, NULL, NULL, NULL);
	
	// Expand to ensure we use 24-bit for RGB and 32-bit for RGBA.
	if (color_type == PNG_COLOR_TYPE_PALETTE || (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8))
	{
		png_set_expand(png_ptr);
	}
	
	// Expand greyscale to rgb.
	if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	{
		png_set_gray_to_rgb(png_ptr);
	}

	// Do we have alpha channel?
	bool has_alpha = ((color_type & PNG_COLOR_MASK_ALPHA) != 0);

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

	// Calculate pitch.	
	TextureFormat::Type format;
	int bytes_per_pixel = 0;
	if (has_alpha == true)
	{
		format = TextureFormat::R8G8B8A8;
		bytes_per_pixel = 4;
	}
	else
	{
		format = TextureFormat::R8G8B8;
		bytes_per_pixel = 3;
	}

	int pitch_bytes = png_get_rowbytes(png_ptr, info_ptr);
	//pitch_bytes += 3 - ((pitch_bytes - 1) % 4); // Align to 4-byte

	// Allocate texture memory.
	png_byte* image_data;
	image_data = (png_byte*)platform_malloc(pitch_bytes * temp_height * sizeof(png_byte) + 15);
	DBG_ASSERT(image_data != NULL);

	// Read in each row.
	png_bytep* row_pointers = (png_bytep*)platform_malloc(temp_height * sizeof(png_bytep));
	DBG_ASSERT(row_pointers != NULL);

	for (int i = 0; i < temp_height; i++)
    {
        row_pointers[temp_height - 1 - i] = image_data + i * pitch_bytes;
    }

	// Read in all the image data.
	png_read_image(png_ptr, row_pointers);
	
	// Create texture.
	Texture* texture = Renderer::Get()->Create_Texture((char*)image_data, temp_width, temp_height, pitch_bytes / bytes_per_pixel, format);
	if (texture == NULL)
	{
		platform_free(row_pointers);
		platform_free(image_data);

        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

		delete stream;
		return NULL;
	}

	// Cleanup.	
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	delete stream;

	// Return texture.
	return texture;
}
