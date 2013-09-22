// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#version 120

// gl_FragData[0] = Diffuse
// gl_FragData[1] = Normal
// gl_FragData[2] = Material
// gl_FragData[3] = Position

void main()
{
	gl_FragData[0] = vec4(0.0, 0.0, 0.0, 0.0);
	gl_FragData[1] = vec4(0.0, 0.0, 0.0, 0.0);
	gl_FragData[2] = vec4(0.0, 0.0, 0.0, 0.0);
	gl_FragData[3] = vec4(0.0, 0.0, 0.0, 0.0);
}
