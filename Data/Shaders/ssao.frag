 // ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

// Credit to: http://devmaster.net/posts/3095/shader-effects-screen-space-ambient-occlusion
// Worked out most of this from that!

uniform sampler2D g_diffuse;
uniform sampler2D g_depth;
uniform sampler2D g_normal;
uniform sampler2D g_material;
uniform sampler2D g_noise;
uniform sampler2D g_position;

uniform vec3 g_resolution;

uniform vec3 g_noise_texture_size;

const vec3  g_attenuation 	= vec3(1.0, 0.0, 0.0);
const float g_occluder_bias = 0.0005;
const float g_sample_radius = 30.0;
const int	g_iterations	= 4;
const float	g_itensity		= 1.0;
	
const vec2 g_kernels[4] = vec2[4]
(
	vec2(0, 1),
	vec2(1, 0),
	vec2(0, -1),
	vec2(-1, 0)
);

const float g_sin45 = 0.707107;  

vec2 get_random(vec2 uv)
{
	vec2 noise = texture2D(g_noise, g_resolution.xy * uv / g_noise_texture_size.xy).xy;	
	return normalize(noise * 2.0f - 1.0f);
}
  
float get_depth(vec2 uv)
{
	return texture2D(g_position, uv).w;
}

vec3 get_normal(vec2 uv)
{
	return texture2D(g_normal, uv).xyz;
}

vec3 get_position(vec2 uv)
{
	return texture2D(g_position, uv).xyz;	
}

float get_active(vec2 uv)
{
	return texture2D(g_diffuse, uv).w;
}
 
float calculate_ao(vec3 src, vec3 normal, vec2 uv)
{
	vec3 	dst			= get_position(uv);
	float	active		= get_active(uv);

	// If uv was not drawn to this frame, we have no occlusion.
	if (active <= 0.0)
	{
		return 0.0;
	}
	
	vec3	vec			= dst - src;
	float 	intensity	= max(dot(normalize(vec), normal) - g_occluder_bias, 0.0);

	float 	dist 		= length(src);
	float	attenuation	= 1.0 / (g_attenuation.x + (g_attenuation.y * dist));
	
	return (intensity * g_itensity) * attenuation;
}

void main(void)
{   
	vec2	uv				= gl_TexCoord[0].st;
    vec3 	normal 			= get_normal(uv);	
	vec3 	position		= get_position(uv);	
	vec2	random			= get_random(uv);
	float	depth			= get_depth(uv);
	
	float	radius			= g_sample_radius;// * (1.0 - depth);
	float 	occlusion 		= 0;
	
	vec2	texel_size		= vec2(1.0 / g_resolution.x, 1.0 / g_resolution.y);
	
	// Calculate the occlusion value of this pixel.
	for (int j = 0; j < g_iterations; j++)
	{
		vec2 coord1 = reflect(g_kernels[j], random);
		vec2 coord2 = vec2(coord1.x * g_sin45 - coord1.y * g_sin45,
						   coord1.x * g_sin45 + coord1.y * g_sin45);
						   
		coord1 *= texel_size;
		coord2 *= texel_size;
						   
		occlusion += calculate_ao(position, normal, uv + coord1 * radius);
		occlusion += calculate_ao(position, normal, uv + coord2 * radius * 0.75);
		occlusion += calculate_ao(position, normal, uv + coord1 * radius * 0.5);
		occlusion += calculate_ao(position, normal, uv + coord2 * radius * 0.25);
	}
	occlusion /= g_iterations * 4.0;
	occlusion = clamp(occlusion, 0.0, 1.0);
	
	// Calculate AO factor.
	float ao_factor = occlusion;
	gl_FragData[0] = vec4(ao_factor, ao_factor, ao_factor, 1.0);
}