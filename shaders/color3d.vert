
#version 120

attribute vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 mvp;

uniform vec4 color;
//attribute vec3 normalIn;
//varying vec3 normalOut;

void main(void)
{
	//gl_Position = projection * (view * (model * gl_Vertex));
	//gl_Position = projection * gl_Vertex;
	//gl_Position = projection * view * gl_Vertex;
	//gl_Position = projection * view * model * gl_Vertex;
	//gl_Position = projection * view * model * position;
	gl_Position = mvp * position;
	//gl_Position = position * mvpmat;
	//gl_Position = position * model * view * projection;
	//normalOut = normalIn;
	//gl_Position.x = gl_Vertex.x / 1000.0 / 1.0;
	//gl_Position.y = gl_Vertex.y / 1000.0 / 1.0;
	gl_Position.w = 1;
}
