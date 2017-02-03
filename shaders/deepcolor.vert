#version 120

uniform float width;
uniform float height;

varying float zdepth;

void main(void)
{

	int spdepth = int(gl_Vertex.z);
	float maxrange = 60 * 1000;
	zdepth = float(spdepth) / maxrange;

	gl_Position = vec4(gl_Vertex.x * 2.0 / width - 1.0,
		gl_Vertex.y * -2.0 / height + 1.0,
		zdepth, 
		//gl_Vertex.z,
		//0.000001, 
		1.0);
}
