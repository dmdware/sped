
#version 120

uniform vec4 color;
const vec4 owncolor = vec4(1,1,1,1);

uniform sampler2D texture0;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
uniform sampler2D specularmap;
uniform sampler2D normalmap;
uniform sampler2D shadowmap;
uniform sampler2D ownermap;

varying vec4 lpos;
varying vec3 light_vec;
varying vec3 light_dir;

//varying vec2 texCoordOut0;

varying vec3 normalOut;

varying vec3 eyevec;
//varying float elevtransp;
uniform float maxelev;
varying float elevy;

vec2 poissonDisk[4];

void main (void)
{
  poissonDisk[0] = vec2( -0.94201624, -0.39906216 );
  poissonDisk[1] = vec2( 0.94558609, -0.76890725 );
  poissonDisk[2] = vec2( -0.094184101, -0.92938870 );
  poissonDisk[3] = vec2( 0.34495938, 0.29387760 );

	if(elevy > maxelev)
		discard;

	vec4 texel0 = texture2D(texture0, gl_TexCoord[0].xy);
	//vec4 texel1 = texture(texture1, texCoordOut0);
	//vec4 texel2 = texture(texture2, texCoordOut0);
	//vec4 texel3 = texture(texture3, texCoordOut0);

	if(texel0.w <= 0.1)
		discard;

	float cosTheta = dot( normalOut, light_vec );
	float shadow_bias = 0.005 * tan(acos(cosTheta)); 
	// cosTheta is dot( n,l ), clamped between 0 and 1
	shadow_bias = clamp(shadow_bias, 0, 0.01);
	//shadow_bias = 0;

	vec3 smcoord = lpos.xyz / lpos.w;
	//vec3 smcoord = lpos.xyz;
	//float shadow = max(0.6, 
	//	float(smcoord.z - shadow_bias <= texture(shadowmap, smcoord.xy).x));
	//	float(smcoord.z <= texture(shadowmap, smcoord.xy).x));

	//gl_FragColor = vec4(lpos.x, lpos.y, 0, 1);
	//gl_FragColor = vec4(0, lpos.y, 0, 1);
	//gl_FragColor = vec4(lpos.x, 0, 0, 1);
	//gl_FragColor = vec4(smcoord.x, smcoord.y, 0, 1);
	//gl_FragColor = vec4(smcoord.x, 0, 0, 1);
	//gl_FragColor = vec4(0, smcoord.y, 0, 1);
	//gl_FragColor = vec4(lpos.w/100.0, lpos.w/100.0, lpos.w/100.0, 1);
	//return;

	float shadow = 1;

	for (int i=0;i<4;i++)
	{
  		if ( texture2D( shadowmap, smcoord.xy + poissonDisk[i]/700.0 ).z 
			<  smcoord.z - shadow_bias )
		{
    			shadow-=0.1;
  		}
	}

	//vec3 smcoord = lpos.xyz / lpos.w;
	//float shadow = max(0.6, float(smcoord.z <= texture(shadowmap, smcoord.xy).x));
	//float shadow = 1;

	vec3 bump = normalize( texture2D(normalmap, gl_TexCoord[0].xy).xyz * 2.0 - 1.0);

	//vec3 lvec = normalize(light_vec);
	//float diffuse = max(dot(-lvec, normalOut), 0.0) + 0.50;

	float distSqr = dot(light_vec, light_vec);
	vec3 lvec = light_vec * inversesqrt(distSqr);
	float diffuse = max( dot(lvec, bump), 0.0 ) * 0.75 + 0.50;
	//float diffuse = max( dot(lvec, bump), 0.0 );
	//float diffuse = max( dot(lvec, vec3(0,0,1)), 0.0 ) * 0.75 + 0.50;

	vec3 vvec = normalize(eyevec);
	float specular = pow(clamp(dot(reflect(-lvec, bump), vvec), 0.0, 1.0), 0.7 );
	//vec3 vspecular = vec3(0,0,0);
	vec3 vspecular = texture2D(specularmap, gl_TexCoord[0].xy).xyz * specular;

	//float alph1 = texel1.w;
	//float alph2 = texel2.w;
	//float alph3 = texel3.w;

	vec4 texel1 = texture2D(ownermap, gl_TexCoord[0].xy);
	float alph1 = texel1.w;
	vec4 stexel = texel0;
	stexel = vec4(stexel.xyz * (1.0 - alph1) + owncolor.xyz * alph1, texel0.w);

	//vec4 stexel = texel0;
	//stexel = vec4(stexel.xyz * (1.0 - alph1) + texel1.xyz * alph1, 1.0);
	//stexel = vec4(stexel.xyz * (1.0 - alph2) + texel2.xyz * alph2, 1.0);
	//stexel = vec4(stexel.xyz * (1.0 - alph3) + texel3.xyz * alph3, 1.0);

	//float alph = color.w * texel0.w * elevtransp;
	//float alph = color.w * texel0.w;
	float alph = texel0.w;
	float minlight = min(shadow, diffuse);
	//float minlight = diffuse;
	//float minlight = shadow;

	// with specular highlights:
	gl_FragColor = vec4(color.xyz * stexel.xyz * minlight + vspecular, alph);

	// without specular highlights:
	//gl_FragColor = vec4(color.xyz * stexel.xyz * minlight, alph);

	//gl_FragColor = vec4(color.xyz * stexel.xyz * minlight + vspecular, alph);
	//gl_FragColor = vec4(color.xyz * minlight, alph);
	//gl_FragColor = vec4(color.xyz * stexel.xyz * diffuse + vspecular, alph);
	//gl_FragColor = vec4(normalOut, alph);
	//gl_FragColor = vec4(normalize(light_vec), alph);
	//gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = texel0;
	//gl_FragColor = vec4(normalize(light_vec), color.w * texel0.w);	
	//gl_FragColor = vec4(vspecular, color.w * texel0.w);
	//gl_FragColor = vec4(eyevec, color.w * texel0.w);
}

