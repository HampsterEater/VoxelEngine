// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

uniform sampler2D		g_position;
uniform sampler2D		g_shadow_map;

uniform mat4 			g_light_view_matrix;
uniform mat4 			g_light_world_view_projection_matrix;
uniform mat4 			g_world_view_projection_matrix;
uniform mat4			g_inverse_world_view_matrix;
uniform mat4			g_inverse_world_view_projection_matrix;

void main()
{
	// Ok, prepare for some ballache. We first need to reconstruct the world position
	// of this fragment.
	vec4 view_space_position  = vec4(texture2D(g_position, gl_TexCoord[0].xy).xyz, 1.0);
	vec4 world_space_position = g_inverse_world_view_matrix * view_space_position;
	
	// Now we need to reproject it into the lights projection space. We bias it so we get a result
	// thats between 0-1 not -1 and 1.
	mat4 bias = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 0.5, 0.0,
		0.5, 0.5, 0.5, 1.0	
	);
	vec4 light_position    = g_light_world_view_projection_matrix * world_space_position; 
	vec4 light_uv_position = bias * light_position;

	// Get the shadow depth at this screen point.
	vec4  shadow_color = texture2D(g_shadow_map, light_uv_position.xy);
	float shadow_depth = shadow_color.w;

	// Get the scene depth at this screen point.
	float depth 	   = light_position.z;

	// Can light not see this fragment?
	float visibility   = 1.0;	
	if (light_uv_position.x >= 0 && light_uv_position.x < 1 &&
		light_uv_position.y >= 0 && light_uv_position.y < 1)
	{
		float acne_bias = 0.001;
		if (shadow_depth < depth - acne_bias)
		{
			visibility = 0.0;
		}	
	}
	else
	{
		visibility = 0.5;
	}
	
	// Doooone
	/*if (gl_TexCoord[0].x <= 0.33)
	{
		gl_FragData[0] = vec4(depth, depth, depth, 1.0);
	}
	else if (gl_TexCoord[0].x <= 0.66)
	{
		gl_FragData[0] = vec4(shadow_depth, shadow_depth, shadow_depth, 1.0);
	}
	else
	{*/
	//	gl_FragData[0] = vec4(light_uv_position.xy, 1.0, 1.0);
		gl_FragData[0] = vec4(visibility, visibility, visibility, 1.0);
	//}
}
