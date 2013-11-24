// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

// Constructs the gbuffer's textures! All the fun happens here!

// gl_FragData[0] = Diffuse
// gl_FragData[1] = Normal
// gl_FragData[2] = Material
// gl_FragData[3] = Position

varying vec4 g_vertex_position;
varying vec3 g_vertex_normal;
varying vec4 g_vertex_color;
varying float g_vertex_depth;
varying float g_vertex_depth_nonlinear;

uniform float g_material_shininess;
uniform vec3  g_material_specular;

void main()
{
	gl_FragData[0] = g_vertex_color;
	gl_FragData[1] = vec4(normalize(g_vertex_normal.xyz), 1.0);
	gl_FragData[2] = vec4(g_material_specular, g_material_shininess);
	gl_FragData[3] = vec4(g_vertex_position.xyz, g_vertex_depth);
}
