// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_MATERIAL_
#define _ENGINE_MATERIAL_

#include "Generic\Types\Vector3.h"

class Texture;

// Defines the properties of a material being taken as an input to the renderer.
class Material
{
private:
	Texture*	m_texture;
	float		m_shininess;
	Vector3		m_specular;

public:
	Material(Texture* texture);
	Material(Texture* texture, float shininess);
	Material(Texture* texture, float shininess, Vector3 specular);

	Texture* Get_Texture();
	void Set_Texture(Texture* texture);

	float Get_Shininess();
	void Set_Shininess(float shininess);

	Vector3 Get_Specular();
	void Set_Specular(Vector3 specular);

};

#endif

