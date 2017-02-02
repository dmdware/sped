

#version 120

uniform vec4 color;

uniform sampler2D texture0;
//uniform sampler2D texture1;
//uniform sampler2D texture2;
//uniform sampler2D texture3;
//uniform sampler2D specularmap;
//uniform sampler2D normalmap;
//uniform sampler2D shadowmap;
//uniform sampler2D ownermap;

uniform sampler2D jumptex;
uniform sampler2D posxtex;
uniform sampler2D posytex;
uniform sampler2D posztex;

varying vec4 lpos;
varying vec3 light_vec;
varying vec3 light_dir;

varying vec2 texCoordOut0;

varying vec3 normalOut;

varying vec3 eyevec;


uniform vec4 cornera;
uniform vec4 cornerb;
uniform vec4 cornerc;
uniform vec4 cornerd;

varying vec4 corneraout;
varying vec4 cornerbout;
varying vec4 cornercout;
varying vec4 cornerdout;

uniform float orjplwpx;
uniform float orjplhpx;
uniform float orjlons;
uniform float orjlats;

uniform float orjlon;
uniform float orjlat;

varying vec4 outpos;

uniform int ormapsz;	//widths and heights of diffuse and position textures

void main (void)
{

	vec4 corneraout2 = corneraout / corneraout.w;
	vec4 cornerbout2 = cornerbout / cornerbout.w;
	vec4 cornercout2 = cornercout / cornercout.w;
	vec4 cornerdout2 = cornerdout / cornerdout.w;

	//vec4 pos = gl_FragCoord;

	vec4 outpos2 = outpos / outpos.w;
	//pos = pos / pos.w;

	vec3 updir = vec3( corneraout2 - cornerdout2 );
	vec3 sidedir = vec3( cornercout2 - cornerdout2 );

	updir = normalize( updir );
	sidedir = normalize( sidedir );

	vec3 viewdir = - cross( sidedir, updir );
	
	viewdir = normalize( viewdir );


	float totuplen = length( vec3(corneraout2 - cornerdout2) );
	float totsidelen = length( vec3(cornercout2 - cornerdout2) );

	float uplen = dot(updir, vec3(outpos - cornerdout2) );
	float sidelen = dot(sidedir, vec3(outpos - cornerdout2) );

	uplen = uplen / totuplen;
	sidelen = sidelen / totsidelen;

//	gl_FragColor.x = outpos2.x;
//	gl_FragColor.y = outpos2.y;
//	gl_FragColor.z = outpos2.z;

	//gl_FragColor.x = orjlon;
	//gl_FragColor.y = sidelen;
	//gl_FragColor.y = orjlat;

	//gl_FragColor.x = pos.x * 0.5 + 0.5;
	//gl_FragColor.y = pos.z * 0.5 + 0.5;

	//jump tex pixel size
	float pltxw = orjplwpx * orjlons;
	float pltxh = orjplhpx * orjlats;

	//plane angle
	//can only determine viewing angle from CPU side, 
	//outside of screen-space coords, because
	//the plane quad is always drawn head-on.
	vec2 pla = vec2(
			orjlon,
			orjlat
			//0.5 + atan(viewdir.z, viewdir.x) / (2.0 * 3.1415),//lon
			//0.5 - asin(viewdir.y)/3.1415//lat
			);
	//float lon ;
	//float lat ;
	//ratio, not rad
	//to int
	pla.x = floor( pla.x * orjlons );
	pla.y = floor( pla.y * orjlats );

	//plane pixel coord
	vec2 plc = vec2(	//to int
			floor(sidelen * orjplwpx),
			floor(uplen * orjplhpx)
			);

//	vec2 jumptc = vec2(sidelen, uplen);

	//jump table index coords
	float tabx = plc.x + pla.x * orjplwpx;
	float taby = plc.y + pla.y * orjplhpx;
	//to (0,1) ratio
	tabx = tabx / pltxw;
	taby = taby / pltxh;

	vec2 jumptc = vec2( tabx, taby );


	vec4 jumpel = texture2D(jumptex, jumptc);

//assume little endian
	int jumpi = int(jumpel.x*255) + 
		int(jumpel.y*255) * 256 +
		int(jumpel.z*255) * 256 * 256;

	vec2 jumpc = vec2( 
			float( (jumpi%ormapsz) ) / float(ormapsz), 
			float( (jumpi/ormapsz) ) / float(ormapsz) );

	vec4 diffel = texture2D(texture0, jumpc.xy);
	vec4 posxel = texture2D(posxtex, jumpc.xy);
	vec4 posyel = texture2D(posytex, jumpc.xy);
	vec4 poszel = texture2D(posztex, jumpc.xy);

//empty space
//	if(jumpel.z > 0.99)
//		discard;
//wrong
		

	vec4 texel0 = texture2D(texture0, gl_TexCoord[0].xy);
	//vec4 texel1 = texture(texture1, texCoordOut0);
	//vec4 texel2 = texture(texture2, texCoordOut0);
	//vec4 texel3 = texture(texture3, texCoordOut0);

	texel0 = diffel;

//	texel0 = vec4(viewdir.xyz, 1);

	//if(texel0.w <= 0.1)
	//	discard;

	gl_FragColor = vec4(texel0.xyz, 1.0);
	//gl_FragColor.z = 1;
	//gl_FragColor.w = 1;
	//gl_FragColor.x = uplen;
	//gl_FragColor.y = sidelen;

//	gl_FragColor = vec4(1,0,0,1);
	//gl_FragColor = texel0;
	//gl_FragColor = vec4(light_vec, color.w * texel0.w);	
	//gl_FragColor = vec4(vspecular, color.w * texel0.w);
	//gl_FragColor = vec4(eyevec, color.w * texel0.w);
}

