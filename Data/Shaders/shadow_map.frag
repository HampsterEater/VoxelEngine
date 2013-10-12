// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

varying float g_depth;

void main()
{
	gl_FragData[0] = vec4(g_depth, g_depth, g_depth, 1.0);
}
