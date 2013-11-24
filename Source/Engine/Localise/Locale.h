// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_LOCALISE_LOCALE_
#define _ENGINE_LOCALISE_LOCALE_

#include "Generic\Types\Vector3.h"
#include "Generic\Types\HashTable.h"
#include "Generic\Patterns\Singleton.h"

#include "Generic\ThirdParty\RapidXML\rapidxml.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_iterators.hpp"
#include "Generic\ThirdParty\RapidXML\rapidxml_utils.hpp"

#include "Engine\Resources\Reloadable.h"

#include <string>

class Texture;
class TextureHandle;

// Quick macro to get a localised string.
#define S(id) (Locale::Get()->Get_String((id)))

class Language : public Reloadable
{
public:
	HashTable<const char*, unsigned int>	Strings;

	rapidxml::xml_document<>*				XML_Document;
	char*									Source_Buffer;

	const char*								Short_Name;
	const char*								Long_Name;

	const char*								URL;

public:
	Language();
	~Language();

	void Reload();
	bool Load(const char* url);

	bool Unpack();

};

class Locale : public Singleton<Locale>
{
private:
	HashTable<Language*, unsigned int> m_languages;
	Language* m_current_language;

public:
	Locale();
	~Locale();

	bool Load_Language(const char* name);
	bool Change_Language(const char* name);
	
	const char* Get_String(std::string& id);
	const char* Get_String(const char* id);
	const char* Get_String(int id);

};

#endif

