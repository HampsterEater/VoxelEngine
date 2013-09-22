// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef SHADER_UNIFORM
#error "SHADER_UNIFORM must be defined when including this file!"
#endif

// These define all the different built-in shader uniform types that can be declared in the render
// pipeline configuration file.

//			Type				Name									Constant Identifier

// Samplers
SHADER_UNIFORM(Texture,		"MATERIAL_TEXTURE",							MaterialTexture)

// Floats
SHADER_UNIFORM(Float,		"CAMERA_NEAR_CLIP",							CameraNearClip)
SHADER_UNIFORM(Float,		"CAMERA_FAR_CLIP",							CameraFarClip)
SHADER_UNIFORM(Float,		"CAMERA_FOV",								CameraFOV)
SHADER_UNIFORM(Float,		"MATERIAL_SHININESS",						MaterialShininess)
SHADER_UNIFORM(float,		"LIGHT_RADIUS",								LightRadius)
SHADER_UNIFORM(float,		"LIGHT_OUTER_RADIUS",							LightOuterRadius)

// Ints
SHADER_UNIFORM(int,			"LIGHT_TYPE",								LightType)

// Vector3's
SHADER_UNIFORM(Vector3,		"MATERIAL_SPECULAR",						MaterialSpecular)
SHADER_UNIFORM(Vector3,		"CAMERA_POSITION",							CameraPosition)
SHADER_UNIFORM(Vector3,		"CAMERA_DIRECTION",							CameraDirection)
SHADER_UNIFORM(Vector3,		"RESOLUTION",								Resolution)
SHADER_UNIFORM(Vector4,		"LIGHT_POSITION",							LightPosition)
SHADER_UNIFORM(Vector4,		"LIGHT_DIRECTION",							LightDirection)
SHADER_UNIFORM(Vector4,		"LIGHT_COLOR",								LightColor)
SHADER_UNIFORM(Vector3,		"TEXTURE_SIZE",								TextureSize)

// Matrix4's
SHADER_UNIFORM(Matrix4,		"WORLD_MATRIX",								WorldMatrix)
SHADER_UNIFORM(Matrix4,		"VIEW_MATRIX",								ViewMatrix)
SHADER_UNIFORM(Matrix4,		"PROJECTION_MATRIX",						ProjectionMatrix)
SHADER_UNIFORM(Matrix4,		"WORLD_VIEW_MATRIX",						WorldViewMatrix)
SHADER_UNIFORM(Matrix4,		"WORLD_VIEW_PROJECTION_MATRIX",				WorldViewProjectionMatrix)
SHADER_UNIFORM(Matrix4,		"NORMAL_MATRIX",							NormalMatrix)

SHADER_UNIFORM(Matrix4,		"INVERSE_WORLD_MATRIX",						InverseWorldMatrix)
SHADER_UNIFORM(Matrix4,		"INVERSE_VIEW_MATRIX",						InverseViewMatrix)
SHADER_UNIFORM(Matrix4,		"INVERSE_PROJECTION_MATRIX",				InverseProjectionMatrix)
SHADER_UNIFORM(Matrix4,		"INVERSE_WORLD_VIEW_MATRIX",				InverseWorldViewMatrix)
SHADER_UNIFORM(Matrix4,		"INVERSE_WORLD_VIEW_PROJECTION_MATRIX",		InverseWorldViewProjectionMatrix)
SHADER_UNIFORM(Matrix4,		"INVERSE_NORMAL_MATRIX",					InverseNormalMatrix)
