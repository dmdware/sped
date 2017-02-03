
#version 120

attribute vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 mvp;

uniform float mind;
uniform float maxd;

attribute vec2 texCoordIn0;
varying vec2 texCoordOut0;

varying float elevy;

void main(void)
{
	//gl_Position = projection * view * model * gl_Vertex;
	//gl_Position = projection * view * gl_Vertex;
	//gl_Position = vec4(0,0,0,1);
	//gl_Position.w = 1;
	gl_Position = mvp * gl_Vertex;
	gl_Position.w = 1;

	//elevy = gl_Vertex.y;
	elevy = (model * gl_Vertex).y;

	gl_TexCoord[0] = gl_MultiTexCoord0;
}
