// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

uniform mat4 g_world_view_matrix;
uniform mat4 g_world_view_projection_matrix;

uniform float g_camera_near_clip;
uniform float g_camera_far_clip;

varying vec4 g_vertex_position;
varying vec3 g_vertex_normal;
varying vec4 g_vertex_color;
varying float g_vertex_depth;
varying float g_vertex_depth_nonlinear;

void main()
{
	gl_Position 			= g_world_view_projection_matrix * gl_Vertex; 
	gl_TexCoord[0] 			= gl_MultiTexCoord0;
	
	// Vertex color!
	g_vertex_color			= gl_Color;
	
	// Get normal in screen space.
	g_vertex_normal   		= mat3(g_world_view_matrix) * gl_Normal;

	// Get position in screen space.
	g_vertex_position   	= g_world_view_matrix * gl_Vertex;
     
	// Get linear depth.
	g_vertex_depth       	= (-g_vertex_position.z - g_camera_near_clip) / (g_camera_far_clip - g_camera_near_clip); 
}