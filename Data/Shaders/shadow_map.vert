// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

uniform mat4 g_light_view_matrix;
uniform mat4 g_light_world_view_projection_matrix;

varying float g_depth;

void main()
{
	gl_Position    = g_light_world_view_projection_matrix * gl_Vertex; 
	gl_TexCoord[0] = gl_MultiTexCoord0;
	
	g_depth = gl_Vertex.z;
}