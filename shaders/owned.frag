
#version 120

uniform vec4 color;
uniform vec4 owncolor;

uniform sampler2D texture0;
uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
uniform sampler2D shadowMap;

varying vec4 lpos;
varying vec3 light_vec;
varying vec3 light_dir;

varying vec2 texCoordOut0;

varying vec3 normalOut;

void main (void)
{
	vec3 smcoord = lpos.xyz / lpos.w;
	float shadow = max(0.5, float(smcoord.z <= texture2D(shadowMap, smcoord.xy).x));

	vec3 lvec = normalize(light_vec);
	float diffuse = max(dot(-lvec, normalOut), 0.0) * 0.75 + 0.25;

	vec4 texel0 = texture2D(texture0, texCoordOut0);
	vec4 texel1 = texture2D(texture1, texCoordOut0);

	float alph1 = texel1.w;

	vec4 stexel = texel0;
	stexel = vec4(stexel.xyz * (1.0 - alph1) + owncolor.xyz * alph1, 1.0);

	gl_FragColor = vec4(color.xyz * stexel.xyz * shadow * diffuse, color.w * texel0.w);
	//gl_FragColor = vec4(texel0.xyz, texel0.w);
	//gl_FragColor = vec4(1, 0, 0, 1);
}

