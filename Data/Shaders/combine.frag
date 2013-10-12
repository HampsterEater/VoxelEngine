// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

// Deferred shading, used to process and render lighting.

uniform sampler2D g_diffuse;
uniform sampler2D g_light_accumulation;
uniform sampler2D g_ssao;
uniform sampler2D g_shadow_map;
uniform sampler2D g_shadow_accumulation;

void main()
{    
    vec3  diffuse 		= texture2D(g_diffuse, gl_TexCoord[0].xy).rgb;	
    vec3  light   		= texture2D(g_light_accumulation, gl_TexCoord[0].xy).rgb;	
    float ssao    		= texture2D(g_ssao, gl_TexCoord[0].xy).x;	
    vec3  shadow  		= texture2D(g_shadow_accumulation, gl_TexCoord[0].xy).rgb;	
    vec3  shadow_map  	= texture2D(g_shadow_map, gl_TexCoord[0].xy).rgb;	

	// Create color by subtracting SSAO buffer from diffuse.
	vec3 color = clamp(diffuse - ssao, 0.0, 1.0);
	
	// Apply lighting.
	color *= light;
	
	// Apply shadow.
	//color *= shadow_map;
	
	// Apply gamma correction
	gl_FragColor.xyz = pow(color, vec3(1.0 / 2.2));
	gl_FragColor.w = 1.0;
	
	//gl_FragColor = vec4(shadow,  1.0); 
}