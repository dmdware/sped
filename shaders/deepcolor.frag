#version 120

#extension GL_EXT_gpu_shader4 : enable

uniform vec4 color;
varying float zdepth;

void main(void)
{
	//gl_FragDepth = gl_FragCoord.z;
	//gl_FragDepth = 0.001;
	gl_FragColor = color;
	gl_FragDepth = zdepth;
}
