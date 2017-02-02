
#version 120

uniform vec4 color;

uniform sampler2D texture0;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
uniform sampler2D specularmap;
uniform sampler2D normalmap;
uniform sampler2D shadowmap;

varying vec4 lpos;
varying vec3 light_vec;
varying vec3 light_dir;

varying vec2 texCoordOut0;

varying vec3 normalOut;

varying vec3 eyevec;
//varying float elevtransp;
uniform float maxelev;
//varying float elevy;

uniform sampler2D gradienttex;
uniform sampler2D detailtex;

void main (void)
{
	//if(elevy > maxelev)
	//	discard;

	vec3 smcoord = lpos.xyz / lpos.w;
	float shadow = max(0.6, float(smcoord.z <= texture2D(shadowmap, smcoord.xy).x));
	//float shadow = 1;

	vec3 bump = normalize( texture2D(normalmap, texCoordOut0).xyz * 2.0 - 1.0);

	//vec3 lvec = normalize(light_vec);
	//float diffuse = max(dot(-lvec, normalOut), 0.0) + 0.50;

	float distSqr = dot(light_vec, light_vec);
	vec3 lvec = light_vec * inversesqrt(distSqr);
	float diffuse = max( dot(lvec, bump), 0.0 ) * 0.75 + 0.50;

	vec3 vvec = normalize(eyevec);
	float specular = pow(clamp(dot(reflect(-lvec, bump), vvec), 0.0, 1.0), 0.7 );
	//vec3 vspecular = vec3(0,0,0);
	vec3 vspecular = texture2D(specularmap, texCoordOut0).xyz * specular;

	vec4 gradtxl = texture2D(gradienttex, texCoordOut0 / 100);
	vec4 dettxl = texture2D(detailtex, texCoordOut0);

	vec4 stexel = vec4(gradtxl.xyz * dettxl.xyz, gradtxl.w * dettxl.w);

	//float alph = color.w * texel0.w * elevtransp;
	float alph = color.w * stexel.w;

	gl_FragColor = vec4(color.xyz * stexel.xyz * shadow * diffuse + vspecular, alph);
	//gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = texel0;
	//gl_FragColor = vec4(light_vec, color.w * texel0.w);	
	//gl_FragColor = vec4(vspecular, color.w * texel0.w);
	//gl_FragColor = vec4(eyevec, color.w * texel0.w);
}

