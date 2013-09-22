// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine\Scene\Light.h"

Light::Light(LightType::Type type)
	: m_type(type)
	, m_radius(1.0f)
	, m_color(Color(255, 255, 255, 255))
{
}

Light::Light(LightType::Type type, float radius)
	: m_type(type)
	, m_radius(radius)
	, m_outer_radius(radius)
	, m_color(Color(255, 255, 255, 255))
{
}

Light::Light(LightType::Type type, float radius, Color color)
	: m_type(type)
	, m_radius(radius)
	, m_color(color)
	, m_outer_radius(radius)
{
}

Light::Light(LightType::Type type, float radius, float outer_radius, Color color)
	: m_type(type)
	, m_radius(radius)
	, m_outer_radius(outer_radius)
	, m_color(color)
{
}

// Get modifiers.
LightType::Type Light::Get_Type()
{
	return m_type;
}

void Light::Set_Type(LightType::Type type)
{
	m_type = type;
}

float Light::Get_Radius()
{
	return m_radius;
}

void Light::Set_Radius(float radius)
{
	m_radius = radius;
}

float Light::Get_Outer_Radius()
{
	return m_outer_radius;
}

void Light::Set_Outer_Radius(float radius)
{
	m_outer_radius = radius;
}

Color Light::Get_Color()
{
	return m_color;
}

void Light::Set_Color(Color color)
{
	m_color = color;
}
