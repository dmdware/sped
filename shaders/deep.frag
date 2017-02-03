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

//problem: you can't read the screen depth. have to keep track of it yourself in a render buffer texture.

//making up for lack of modulus in dedicated video card for this version of glsl (?)
//getting a syntax error otherwise
int modulus(int x, int d)
{
	int y = x / d;
	return x - y*d;
}

void main(void)
{
	vec4 texel = texture2D(texture0, gl_TexCoord[0].xy);

	//if(texel.w <= 0.1)
	//	discard;

	texel = texel * color;
	//texel.w = 1.0;
    
    vec2 screentexcoordratio = vec2(gl_FragCoord) * vec2(width,height) / vec2(screenmapwidth,screenmapheight);
    
    //float t;
    //int test;
    //mac doesn't like uint for some reason
    
    vec4 spdepthel = texture2D(spritedepthmap, gl_TexCoord[0].xy);
    //vec4 spdepthel = texture2D(spritedepthmap, vec2(gl_TexCoord[0].x,height-gl_TexCoord[0].y));
    //vec4 scdepthel = texture2D(screendepthmap, screentexcoordratio);
    
	int byte1,byte2,byte3,byte4;

    //int byte1 = int(scdepthel.x * 255.0);
    //int byte2 = int(scdepthel.y * 255.0);
    //int byte3 = int(scdepthel.z * 255.0);
    //int byte4 = 0;
   // int scdepth = byte1 + byte2 * 256 + byte3 * 256 * 256;
    
    byte1 = int(spdepthel.x * 255.0);
    byte2 = int(spdepthel.y * 255.0);
    byte3 = int(spdepthel.z * 255.0);
    byte4 = 0;
    int spdepth = byte1 + byte2 * 256 + byte3 * 256 * 256;
    
    //if(spdepth >= scdepth)
    //if(spdepth > scdepth)
    //    discard;
    
    //gl_FragCoord.z = float(spdepth) / (256.0 * 256.0 * 256.0);
    //gl_FragCoord.w = 1.0;
    gl_FragDepth = float(spdepth) / (256.0 * 256.0 * 256.0);
    //gl_FragDepth = 0;
    
    //texel.x = float(spdepth) / (256.0 * 256.0 * 256.0);
    //texel.y = float(spdepth) / (256.0 * 256.0 * 256.0);
    //texel.z = float(spdepth) / (256.0 * 256.0 * 256.0);

	//texel.x = float(byte1) / 255.0;
	//texel.y = float(byte2) / 255.0;
	//texel.z = float(byte3) / 255.0;
	texel = spdepthel;


/*
	scdepth = scdepth - 10000;
	//byte1 = scdepth % 256;
	//byte2 = (scdepth / 256) % 256;
	//byte3 = (scdepth / (256 * 256)) % 256;

	byte1 = modulus(scdepth, 256);
	byte2 = modulus((scdepth / 256), 256);
	byte3 = modulus((scdepth / (256 * 256)), 256);

	scdepthel.x = float(byte1) / 255.0;
	scdepthel.y = float(byte2) / 255.0;
	scdepthel.z = float(byte3) / 255.0;
*/

	texel.w = 1.0;
    
	gl_FragColor = texel;
    //gl_FragColor = spdepthel;
}
