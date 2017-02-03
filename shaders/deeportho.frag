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

void main(void)
{
	vec4 texel = texture2D(texture0, gl_TexCoord[0].xy);

	if(texel.w <= 0.1)
		discard;

	texel = texel * color;
	//texel.w = 1.0;
    
    vec2 screentexcoordratio = vec2(gl_FragCoord) * vec2(width,height) / vec2(screenmapwidth,screenmapheight);
    
    //float t;
    //int test;
    //mac doesn't like uint for some reason
    
    vec4 spdepthel = texture2D(spritedepthmap, gl_TexCoord[0].xy);
   // vec4 scdepthel = texture2D(screendepthmap, screentexcoordratio);
    
	int byte1, byte2, byte3, byte4;

    //int byte1 = int(scdepthel.x * 255.0);
    //int byte2 = int(scdepthel.y * 255.0);
    //int byte3 = int(scdepthel.z * 255.0);
    //int byte4 = 0;
    //int scdepth = byte1 + byte2 * 256 + byte3 * 256 * 256;

	//int speddepth = 28 + 256 * 200 + 256 * 256 * 7;

    byte1 = int(spdepthel.x * 255.0);
    byte2 = int(spdepthel.y * 255.0);
    byte3 = int(spdepthel.z * 255.0);
    byte4 = 0;
    //int spdepth = byte1 + byte2 * 256 + byte3 * 256 * 256 - speddepth + int(basedepth);
    int spdepth = byte1 + byte2 * 256 + byte3 * 256 * 256 + int(basedepth);
    
    //if(spdepth >= scdepth)
    //    discard;
    
	//float maxrange = 256.0 * 256.0 * 20.0;
	float maxrange = 60 * 1000;
	//float maxrange = 256.0 * 256.0 * 256.0;

	//float fragdepth;

    //gl_FragCoord.z = float(spdepth) / (256.0 * 256.0 * 256.0);
    //gl_FragCoord.w = 1.0;
    //gl_FragDepth = float(spdepth) / (256.0 * 256.0 * 256.0);
	/*
	dividing by less because floats here can't hold that precise a number
	otherwise, doesn't render right
	as long as it doesn't go over 256x256, shouldn't matter, otherwise starts wrapping around
	we want the fragdepth max to be 1.0,
	so what we divide by should be the max depth in our visible scene.
	dividing by anything over what a 24-bit float can hold precisely will result
	in loss of precision.
	dividing by 256x256x20 seems to render the trees correctly with respect to buildings
	in front of them. BUT this results in roads z-fighting with ground.
	*/
    //gl_FragDepth = float(spdepth) / (256.0 * 256.0 * 1.0);
    gl_FragDepth = float(spdepth) / maxrange;
    //gl_FragDepth = 0;

/*
	const float C = 0.1;
	float FC = 1.0/log(maxrange*C + 1);
 
	logz = log(gl_Position.w*C + 1)*FC;
	gl_Position.z = (2*logz - 1)*gl_Position.w;
*/
    
 /*
    texel.x = 1.0 - float(spdepth) / maxrange;
    texel.y = 1.0 - float(spdepth) / maxrange;
    texel.z = 1.0 - float(spdepth) / maxrange;
   */ 
	gl_FragColor = texel;

	//give a "depth fog" effect to accentuate 3d-ness

	//attempt to emulate distance to ground
	//greater depth = more fog
	//but, higher on screen = less fog, at same depth
/*
	float screenyratio = gl_FragCoord.y / height;
	//float fog = min(1, 3.0 * float(spdepth) / maxrange);
	//float fog = float(spdepth) / maxrange * 3.2 - screenyratio / 2;
	float depratio = float(spdepth) / maxrange;
	//depratio = depratio - 0.1 - screenyratio / 20;
	depratio = max(0, depratio);
	//edit: not really fog, but fogging with color doesn't work so well
	depratio = depratio * 3.2;
	depratio = depratio + (1.0 - screenyratio) / 2.3;
	//depratio = min(1, depratio);
	gl_FragColor = vec4(texel.xyz * (depratio) + vec3(0,0,0) * (1.0-depratio), texel.w);
	//gl_FragColor = vec4(depratio,depratio,depratio,texel.w);
	//gl_FragColor = vec4(texel.xyz * (1.0 - fog) * 4.5 + vec3(0.5,0.5,0.5) * fog, texel.w);
*/

    //gl_FragColor = spdepthel;
}
