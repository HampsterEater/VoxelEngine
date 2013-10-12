// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Resources\ResourceCompiler.h"
#include "Engine\IO\StreamFactory.h"

ResourceCompiler::ResourceCompiler()
{
}

ResourceCompiler::~ResourceCompiler()
{
}

std::string ResourceCompiler::Directory_To_Output_Directory(std::string directory)
{
	Platform* platform = Platform::Get();
	
	std::string path = Platform::Get()->Join_Path(".compiled", directory); 

	if (!platform->Is_Directory(path.c_str()))
	{
		platform->Create_Directory(path.c_str(), true);
	}

	return path;
}

bool ResourceCompiler::Check_File_Timestamp(std::string path)
{
	Platform* platform = Platform::Get();

	std::string input_directory	= platform->Extract_Directory(path);
	std::string input_filename	= platform->Extract_Filename(path);
	std::string output_directory= Directory_To_Output_Directory(input_directory);
	std::string output_path		= platform->Join_Path(output_directory, input_filename + ".timestamp");

	// If input file dosen't exist, we can't compile.
	if (!platform->Is_File(path.c_str()))
	{
		return false;
	}

	// If output file dosen't exist, we need to compile.
	if (!platform->Is_File(output_path.c_str()))
	{
		return true;
	}

	// Check timestamps.
	u64 input_timestamp  = StreamFactory::Get_Last_Modified(path.c_str());
	u64 output_timestamp = StreamFactory::Get_Last_Modified(output_path.c_str());

	return input_timestamp > output_timestamp;
}

void ResourceCompiler::Update_File_Timestamp(std::string path)
{
	Platform* platform = Platform::Get();
	
	std::string input_directory	= platform->Extract_Directory(path);
	std::string input_filename	= platform->Extract_Filename(path);
	std::string output_directory= Directory_To_Output_Directory(input_directory);
	std::string output_path		= platform->Join_Path(output_directory, input_filename + ".timestamp");	
	u64			input_timestamp = StreamFactory::Get_Last_Modified(path.c_str());

	Stream* stream = StreamFactory::Open(output_path.c_str(), StreamMode::Write);
	if (stream == NULL)
	{
		return;
	}

	stream->WriteInt32(input_timestamp);
	stream->Close();

	SAFE_DELETE(stream);
}
