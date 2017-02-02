
//#version 120
#version 130

uniform float maxd;
uniform float mind;

uniform sampler2D texture0;

varying vec2 texCoordOut0;

void main(void)
{
	//gl_FragColor = color * texture2D(texture0, texCoordOut0);
	//gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	//gl_FragColor = vec4(fragCoordZ, fragCoordZ, fragCoordZ, 1.0);
	//gl_FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);

	vec4 texel0 = texture2D(texture0, gl_TexCoord[0].xy);
	//gl_FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, texel0.w);

	if(texel0.w < 0.5)
		discard;

	uint depth = uint( mind + max(0.0, gl_FragCoord.z) * (maxd - mind) );

	const uint maxby = uint(256);

	uint byte1 = depth % maxby;
	uint byte2 = (depth / maxby) % maxby;
	uint byte3 = (depth / (maxby * maxby)) % maxby;
	uint byte4 = (depth / (maxby * maxby * maxby));

	float r = float(byte1) / 255.0;
	float g = float(byte2) / 255.0;
	float b = float(byte3) / 255.0;
	float a = float(byte4) / 255.0;

	//r = 1.0;
	//b = 0.5;
	//g = 0.1;
	a = 1.0;

	gl_FragColor = vec4(r,g,b,a);
	//gl_FragColor = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1.0);
	//gl_FragColor = vec4(0, 0, 1, 1.0);
	//gl_FragDepth = gl_FragCoord.z;
}
