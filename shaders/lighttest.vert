
#version 120

attribute vec4 position;

uniform mat4 mvp;

uniform float width;
uniform float height;

attribute vec2 texCoordIn0;
varying vec2 texCoordOut0;

void main(void)
{
/*
	gl_Position = vec4(gl_Vertex.x * 2.0 / width - 1.0,
		gl_Vertex.y * -2.0 / height + 1.0,
		gl_Vertex.z, 
		1.0);
*/
	gl_Position = mvp * gl_Vertex;
	//gl_Position.w = 1;
                     
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
