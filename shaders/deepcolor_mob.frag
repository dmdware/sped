

//#extension GL_EXT_gpu_shader4 : enable

uniform lowp vec4 color;
varying highp float zdepth;

void main(void)
{
	//gl_FragDepth = gl_FragCoord.z;
	//gl_FragDepth = 0.001;
	gl_FragColor = color;
	//gl_FragDepth = zdepth;
}
