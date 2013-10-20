// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_TEXT_FREETYPE_RESOURCECOMPILER_
#define _ENGINE_TEXT_FREETYPE_RESOURCECOMPILER_

#include "Engine\Resources\ResourceCompiler.h"

#include "Engine\Config\ConfigFile.h"

#include "Generic\ThirdParty\RapidXML\rapidxml.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_iterators.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_utils.hpp"

class FreeType_ResourceCompiler : public ResourceCompiler
{
private:
	std::string m_input_path;
	std::string m_input_directory;
	std::string m_input_filename;
	std::string m_output_directory;
	std::string m_output_path;
	
	ConfigFile* m_config_file;

	const char* m_ttf_input_path;

protected:

	bool Load_Config();

public:

	// Constructors
	FreeType_ResourceCompiler		(const char* url);	
	~FreeType_ResourceCompiler		();	

	// Derived methods.
	bool			Should_Compile		();
	std::string		Get_Compiled_Path	();
	bool			Compile				();
	FreeType_Font*	Load_Compiled		();

};

#endif

