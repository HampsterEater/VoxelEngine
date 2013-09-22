// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

// gbuffer textures.
uniform sampler2D g_depth;
uniform sampler2D g_normal;
uniform sampler2D g_position;
uniform sampler2D g_material;

// Light settings.
uniform vec4  g_light_position;
uniform vec4  g_light_direction;
uniform vec4  g_light_color;
uniform float g_light_radius;
uniform float g_light_outer_radius;
uniform int   g_light_type;

// Light Types:
//		0 = Point
//		1 = Ambient
//		2 = Directional
//		3 = Spotlight

// ===================================================================
//		POINT LIGHT
// ===================================================================
vec4 point_light(vec3 normal, vec3 position, vec3 specular, float shininess)
{
	vec3	specular_output = vec3(0.0);
	
	// Calculate vectors.
	vec3 l = g_light_position.xyz - position;		// light vector
	vec3 v = normalize(position);					// view vector
	vec3 h = normalize(v + l);						// half vector
	
	// attenuation
	float att = clamp(1.0 - length(l) / g_light_radius, 0.0, 1.0);
	l = normalize(l);
	
	// diffuse and specular terms
	vec3 Idiff = clamp(dot(l, normal), 0.0, 1.0) * g_light_color.rgb;
	vec3 Ispec = clamp(pow(dot(h, normal), shininess), 0.0, 1.0) * specular * g_light_color.rgb;
	
	// Final output
	return vec4(att * clamp(Idiff + Ispec, 0.0, 1.0), 1.0);
}

// ===================================================================
//		AMBIENT LIGHT
// ===================================================================
vec4 ambient_light(vec3 normal, vec3 position, vec3 specular, float shininess)
{
	return vec4(g_light_color.rgb, 1.0);
}

// ===================================================================
//		DIRECTIONAL LIGHT
// ===================================================================
vec4 directional_light(vec3 normal, vec3 position, vec3 specular, float shininess)
{
	vec3	specular_output = vec3(0.0);
	
	// Calculate factors.
	vec3 light_vector 	= normalize(g_light_direction.xyz);
	
	// Intensity
	float intensity = max(dot(normal, light_vector), 0.0);
	if (intensity > 0.0)
	{
		vec3 half = normalize(light_vector + position);		
		float specular_intensity = max(dot(half, normal), 0.0);
		specular_output = clamp(specular * pow(specular_intensity, shininess), 0.0, 1.0);
	}
	
	// Final output
	return vec4(vec3(intensity * (g_light_color.xyz + specular_output)), 1.0);
}

// ===================================================================
//		SPOTLIGHT LIGHT
// ===================================================================
vec4 spotlight_light(vec3 normal, vec3 position, vec3 specular, float shininess)
{
	vec3	specular_output = vec3(0.0);

	// Calculate factors.
	vec3 light_direction 	= normalize(g_light_direction.xyz);
	vec3 source_vector 		= normalize(g_light_position.xyz - position);
	vec3 position_vector	= normalize(position.xyz);
	vec3 half_vector		= normalize(position_vector + source_vector);
	float angle				= acos(dot(-source_vector, light_direction));
	float cutoff			= radians(g_light_radius);
	float outer_cutoff		= radians(g_light_outer_radius);
	float intensity			= 0.0;
		
	// In spotlight?
	if (angle < outer_cutoff)
	{
		intensity = 1.0 - clamp((angle - cutoff) / (outer_cutoff - cutoff), 0.0, 1.0);
		
		if (intensity > 0.0)
		{	
			float specular_intensity = max(dot(half_vector, normal), 0.0);
			specular_output = clamp(specular * pow(specular_intensity, shininess), 0.0, 1.0);
		}		
	}

	return vec4(vec3(intensity * (g_light_color.xyz + specular_output)), 1.0);
}

// ===================================================================
//		ENTRY POINT
// ===================================================================
void main()
{   	
    vec3 	normal 			= clamp(texture2D(g_normal, gl_TexCoord[0].xy).rgb, -1.0, 1.0);	
    vec4 	material 		= texture2D(g_material, gl_TexCoord[0].xy);	
	vec3 	position		= texture2D(g_position, gl_TexCoord[0].xy).xyz;
	vec3	specular		= clamp(material.xyz, 0, 1);
	float   shininess		= clamp(material.w, 0, 1);
	
	if (g_light_type == 0)
	{
		gl_FragColor = point_light(normal, position, specular, shininess);
	}
	else if (g_light_type == 1)
	{
		gl_FragColor = ambient_light(normal, position, specular, shininess);
	}
	else if (g_light_type == 2)
	{
		gl_FragColor = directional_light(normal, position, specular, shininess);
	}
	else if (g_light_type == 3)
	{
		gl_FragColor = spotlight_light(normal, position, specular, shininess);
	}	
}