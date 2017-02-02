
#version 120

uniform vec4 color;

varying vec2 texCoordOut0;
uniform sampler2D texture0;

void main(void)
{
	//gl_FragColor = color * texture2D(texture0, gl_TexCoord[0].xy);
	gl_FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
}
