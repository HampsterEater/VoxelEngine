// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

uniform sampler2D g_texture;
uniform vec4 g_color;

void main()
{
	gl_FragColor = texture2D(g_texture, gl_TexCoord[0].st) * g_color;
}