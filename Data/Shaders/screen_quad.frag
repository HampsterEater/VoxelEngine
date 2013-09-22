// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

// Basic fragment shader just renders the scene without 
// any kind of special effects.

uniform sampler2D g_texture;

void main()
{
	gl_FragData[0] = texture2D(g_texture, gl_TexCoord[0].st);
}