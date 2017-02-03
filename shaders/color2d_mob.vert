

uniform float width;
uniform float height;

attribute vec2 position;

void main(void)
{
	gl_Position = vec4(position.x * 2.0 / width - 1.0,
		position.y * -2.0 / height + 1.0,
		0, 
		1.0);
}