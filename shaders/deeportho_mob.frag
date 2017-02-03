
/*
#version 120

#extension GL_EXT_gpu_shader4 : enable

uniform vec4 color;
uniform sampler2D texture0;
uniform sampler2D spritedepthmap;
uniform sampler2D screendepthmap;
uniform float screenmapwidth;
uniform float screenmapheight;
uniform float width;
uniform float height;
uniform float basedepth;
*/

uniform lowp vec4 color;
uniform sampler2D texture0;

varying highp vec2 texcoordout0;


//varying highp float passdepth;

void main(void)
{
	lowp vec4 texel = texture2D(texture0, texcoordout0.xy);

	if(texel.w <= 0.1)
		discard;

	texel = texel * color;
	//texel.w = 1.0;
	
	//texel.xyz = vec3(passdepth,passdepth,passdepth);
	
	gl_FragColor = texel;
}
