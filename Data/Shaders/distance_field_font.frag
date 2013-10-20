// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

uniform sampler2D g_texture;
uniform float g_scale;
uniform float g_spread;

void main()
{
	if (g_scale <= 1.0)
	{
		float distance = texture2D(g_texture, gl_TexCoord[0].st).r;
		gl_FragColor = vec4(vec3(1.0, 1.0, 1.0), distance);
	}
	else
	{
		float distance = texture2D(g_texture, gl_TexCoord[0].st).r;
		float smoothing = 0.25 / (g_spread * g_scale);
		float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);  
		gl_FragColor = vec4(vec3(1.0, 1.0, 1.0), alpha);
	}	
//	gl_FragColor = vec4(vec3(gl_TexCoord[0].s, gl_TexCoord[0].t, 1.0), 1.0);	
}