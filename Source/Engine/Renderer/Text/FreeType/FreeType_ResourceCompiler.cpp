// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Renderer\Text\FreeType\FreeType_FontFactory.h"
#include "Engine\Renderer\Text\FreeType\FreeType_Font.h"
#include "Engine\Renderer\Text\FreeType\FreeType_ResourceCompiler.h"
#include "Engine\Renderer\Textures\TextureFactory.h"
#include "Engine\IO\StreamFactory.h"
#include "Generic\Types\HashTable.h"
#include "Generic\Helper\StringHelper.h"

FreeType_ResourceCompiler::FreeType_ResourceCompiler(const char* url)
	: m_config_file(NULL)
	, m_ttf_input_path(NULL)
{
	Platform*	platform	= Platform::Get();

	m_input_path			= url;
	m_input_directory		= platform->Extract_Directory(url);
	m_input_filename		= platform->Extract_Filename(url);
	m_output_directory		= Directory_To_Output_Directory(m_input_directory);
	m_output_path			= platform->Join_Path(m_output_directory, m_input_filename);
}

FreeType_ResourceCompiler::~FreeType_ResourceCompiler()
{
	SAFE_DELETE(m_config_file);
}

bool FreeType_ResourceCompiler::Should_Compile()
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
	if (Check_File_Timestamp(m_ttf_input_path))
	{
		return true;
	}

	return false;
}

std::string FreeType_ResourceCompiler::Get_Compiled_Path()
{
	return m_output_path;
}

bool FreeType_ResourceCompiler::Load_Config()
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

	// Extract strings.
	 m_ttf_input_path = m_config_file->Get<const char*>("generation/ttf");

	return true;
}

bool FreeType_ResourceCompiler::Compile()
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
	DBG_LOG("Compiling font resource '%s'.", m_input_path.c_str());

	// Open the ttf file.
	Stream* stream = StreamFactory::Open(m_ttf_input_path, StreamMode::Read);
	if (stream == NULL)
	{
		DBG_LOG("Failed to compile font, ttf file could not be found: '%s'", m_ttf_input_path);
		return NULL;
	}

	// Load in the font file.
	int size = stream->Length();
	char* buffer = new char[size];
	if (buffer == NULL)
	{
		SAFE_DELETE(stream);
		return NULL;
	}
	stream->Read(buffer, 0, size);

	// Load in font face.
	FT_Library library = FreeType_FontFactory::Get_FreeType_Library();
	FT_Face face;

	int result = FT_New_Memory_Face(library, (FT_Byte*)buffer, size, 0, &face);
	if (result != 0)
	{
		DBG_LOG("Failed to open freetype font face: %s", m_ttf_input_path);
		SAFE_DELETE(buffer);
		SAFE_DELETE(stream);
		return NULL;
	}

	// Return resulting font!
	SAFE_DELETE(stream);
	FreeType_Font* font = new FreeType_Font(library, face, buffer, m_config_file);

	// Configuration settings.
	ConfigFile config;
	
	// Store configuration settings.
	config.Set<const char*>("generation/ttf",				m_config_file->Get<const char*>("generation/ttf"));
	config.Set<const char*>("generation/sdf-spread",		m_config_file->Get<const char*>("generation/sdf-spread"));
	config.Set<const char*>("generation/sdf-downscale",		m_config_file->Get<const char*>("generation/sdf-downscale"));
	config.Set<const char*>("generation/sdf-threshold",		m_config_file->Get<const char*>("generation/sdf-threshold"));
	config.Set<const char*>("generation/texture-size",		m_config_file->Get<const char*>("generation/texture-size"));
	config.Set<const char*>("generation/max-textures",		m_config_file->Get<const char*>("generation/max-textures"));
	config.Set<const char*>("generation/glyph-size",		m_config_file->Get<const char*>("generation/glyph-size"));
	config.Set<const char*>("generation/glyph-spacing",		m_config_file->Get<const char*>("generation/glyph-spacing"));

	// Save pngs.
	int texture_count = 0;
	FreeType_FontTexture** textures = font->Get_Textures(texture_count);

	for (int i = 0; i < texture_count; i++)
	{
		FreeType_FontTexture* texture = textures[i];		

		std::string output_path = (m_output_path + "." + StringHelper::To_String(i) + ".png");

		// Save texture file.
		TextureFactory::Save(output_path.c_str(), texture->Texture, TextureFlags::None);

		// Store texture spec in xml.
		ConfigFileNode node = config.New_Node("textures/texture");
		config.Set(NULL, output_path.c_str(), node, false);
	}

	// Save glyph configuration.
	HashTable<FreeType_FontGlyph*, unsigned int>& glyphs = font->Get_Glyphs();
	for (HashTable<FreeType_FontGlyph*, unsigned int>::Iterator iter = glyphs.Begin(); iter != glyphs.End(); iter++)
	{
		FreeType_FontGlyph* ft_glyph = iter.Get_Value();
		
		// Store character spec in xml.
		ConfigFileNode node = config.New_Node("glyphs/glyph");
		config.Set("texture",		ft_glyph->TextureIndex,				node, true);
		config.Set("ftindex",		(int)ft_glyph->FreeType_GlyphIndex, node, true);
		config.Set("advance",		ft_glyph->Glyph.Advance,			node, true);
		config.Set("baseline",		ft_glyph->Glyph.Baseline,			node, true);
		config.Set("glyph",			(int)ft_glyph->Glyph.Glyph,			node, true);
		config.Set("offset",		ft_glyph->Glyph.Offset,				node, true);
		config.Set("size",			ft_glyph->Glyph.Size,				node, true);
		config.Set("uv",			ft_glyph->Glyph.UV,					node, true);
	}

	// Save font xml configuration.
	config.Save(m_output_path.c_str());

	// Update timestamps.
	Update_File_Timestamp(m_ttf_input_path);
	Update_File_Timestamp(m_input_path);

	DBG_LOG("Finished compiling to '%s'.", m_output_path.c_str());
	return true;
}

FreeType_Font* FreeType_ResourceCompiler::Load_Compiled()
{
	std::string compiled_path = Get_Compiled_Path();
	Platform* platform = Platform::Get();

	// Compile time.
	DBG_LOG("Loading font resource '%s'.", compiled_path.c_str());
	
	// Load configuration settings.
	ConfigFile config;
	if (!config.Load(compiled_path.c_str()))
	{
		DBG_LOG("Failed to load font, config file could not be found: '%s'", compiled_path.c_str());
		return NULL;
	}

	// Grab configuration path.
	std::string ttf_input_path = config.Get<const char*>("generation/ttf");

	// Open the ttf file.
	Stream* stream = StreamFactory::Open(ttf_input_path.c_str(), StreamMode::Read);
	if (stream == NULL)
	{
		DBG_LOG("Failed to load font, ttf file could not be found: '%s'", ttf_input_path.c_str());
		return NULL;
	}

	// Load in the font file.
	int size = stream->Length();
	char* buffer = new char[size];
	if (buffer == NULL)
	{
		SAFE_DELETE(stream);
		return NULL;
	}
	stream->Read(buffer, 0, size);

	// Load in font face.
	FT_Library library = FreeType_FontFactory::Get_FreeType_Library();
	FT_Face face;

	int result = FT_New_Memory_Face(library, (FT_Byte*)buffer, size, 0, &face);
	if (result != 0)
	{
		DBG_LOG("Failed to open freetype font face: %s", m_ttf_input_path);
		SAFE_DELETE(buffer);
		SAFE_DELETE(stream);
		return NULL;
	}

	// Create the font.
	SAFE_DELETE(stream);
	FreeType_Font* font = new FreeType_Font(library, face, buffer);
	if (font->Load_Compiled_Config(&config))
	{
		SAFE_DELETE(font);
		SAFE_DELETE(stream);
		return NULL;
	}

	DBG_LOG("Finished loading compiled font from '%s'.", compiled_path.c_str());
	return font;
}