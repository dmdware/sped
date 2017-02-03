

uniform float width;
uniform float height;

attribute vec2 position;

void main(void)
{
	float spdepth = basedepth;
	float maxrange = 60 * 1000;
	float zdepth = spdepth / maxrange;

	gl_Position = vec4(position.x * 2.0 / width - 1.0,
		position.y * -2.0 / height + 1.0,
		zdepth, 
		//gl_Vertex.z,
		//0.000001, 
		1.0);
}
