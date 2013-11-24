// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

uniform mat4 g_world_view_projection_matrix;

void main()
{
	gl_Position    = g_world_view_projection_matrix  * gl_Vertex; 
	gl_TexCoord[0] = gl_MultiTexCoord0;
}