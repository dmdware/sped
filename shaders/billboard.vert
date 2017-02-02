
#version 120

//attribute vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;

//attribute vec3 normalIn;
//varying vec3 normalOut;

attribute vec2 texCoordIn0;
varying vec2 texCoordOut0;

void main(void)
{
	gl_Position = projection * (view * (model * gl_Vertex));
	//normalOut = normalIn;
	texCoordOut0 = texCoordIn0;
	gl_Position.w = 1;
}
