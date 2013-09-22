// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

// Deferred shading, used to process and render lighting.

uniform sampler2D g_diffuse;
uniform sampler2D g_depth;
uniform sampler2D g_normal;
uniform sampler2D g_material;
uniform sampler2D g_light_accumulation;
uniform sampler2D g_ssao;

void main()
{    
    vec3  diffuse = texture2D(g_diffuse, gl_TexCoord[0].xy).rgb;	
    vec3  light   = texture2D(g_light_accumulation, gl_TexCoord[0].xy).rgb;	
    float ssao    = texture2D(g_ssao, gl_TexCoord[0].xy).x;	

	// Create color by subtracting SSAO buffer from diffuse.
	vec3 color = clamp(diffuse - ssao, 0.0, 1.0);
	
	// Apply lighting.
	color *= light;
	
	// Apply gamma correction
	gl_FragColor.xyz = pow(color, vec3(1.0 / 2.2));
	gl_FragColor.w = 1.0;
	
	//gl_FragColor = vec4(light, 1.0); 
}