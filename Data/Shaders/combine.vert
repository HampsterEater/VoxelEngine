 // ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

// Deferred shading, used to process and render lighting.

void main()
{
	vec2 position_screen = gl_Vertex.xy;
	gl_Position = vec4(position_screen, 0.0, 1.0);
	gl_TexCoord[0] 	= gl_MultiTexCoord0;
}