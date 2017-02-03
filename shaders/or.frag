

#version 120

#extension GL_EXT_gpu_shader4 : enable


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

uniform mat4 mvp;
uniform mat4 imvp;

void main (void)
{
vec4 texel0;

	//vec4 corneraout2 = corneraout / corneraout.w;
	//vec4 cornerbout2 = cornerbout / cornerbout.w;
	//vec4 cornercout2 = cornercout / cornercout.w;
	//vec4 cornerdout2 = cornerdout / cornerdout.w;

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

	//texel0.x = uplen;
	//texel0.y = sidelen;


//	gl_FragColor.x = outpos2.x;
//	gl_FragColor.y = outpos2.y;
//	gl_FragColor.z = outpos2.z;

	//gl_FragColor.x = orjlon;
	//gl_FragColor.y = sidelen;
	//gl_FragColor.y = orjlat;

	//gl_FragColor.x = pos.x * 0.5 + 0.5;
	//gl_FragColor.y = pos.z * 0.5 + 0.5;

	//jump tex pixel size
	//float pltxw = orjplwpx * orjlons;
	//float pltxh = orjplhpx * orjlats;

	//plane angle
	//can only determine viewing angle from CPU side, 
	//outside of screen-space coords, because
	//the plane quad is always drawn head-on.
	//vec2 pla = vec2(
	//		orjlon,
	//		orjlat
			//0.5 + atan(viewdir.z, viewdir.x) / (2.0 * 3.1415),//lon
			//0.5 - asin(viewdir.y)/3.1415//lat
	//		);

	//texel0.x = pla.x;
	//texel0.y = pla.y;

	//float lon ;
	//float lat ;
	//ratio, not rad
	//to int
	//pla.x = floor( pla.x * orjlons );
	//pla.y = floor( pla.y * orjlats );


	//plane pixel coord
	//vec2 plc = vec2(	//to int
	//		floor(sidelen * orjplwpx),
	//		floor(uplen * orjplhpx)
	//		);

//	vec2 jumptc = vec2(sidelen, uplen);

	//jump table index coords
	int tabx = int(sidelen*(orjplwpx-1)) + int(orjlon*orjplwpx*(orjlons-1));
	int taby = int(uplen*(orjplhpx-1)) + int(orjlat*orjplhpx*(orjlats-1));
	float tabxf = float(tabx)/(orjplwpx * orjlons);
	float tabyf = float(taby)/(orjplhpx * orjlats);
	//float tabx = ( sidelen*(orjplwpx-1) + pla.x*orjplwpx*(orjlons-1) ) / 
	// ( orjplwpx * orjlons );
	//float taby = ( uplen*(orjplhpx-1) + pla.y*orjplhpx*(orjlats-1) ) /
	// ( orjplhpx * orjlats );
	//to (0,1) ratio
	//tabx = tabx / pltxw;
	//taby = taby / pltxh;

	//texel0.x = tabx;
	//texel0.y = taby;

	vec2 jumptc = vec2( tabxf, tabyf );

	vec4 jumpel = texture2D(jumptex, jumptc);

	//assume little endian
	int jumpi = int(jumpel.x*255) + 
		int(jumpel.y*255) * 256 +
		int(jumpel.z*255) * 256 * 256;

	vec2 jumpc = vec2( 
			float( (jumpi%ormapsz) ) / float(ormapsz), 
			float( (jumpi/ormapsz) ) / float(ormapsz) );

	//texel0.x = jumpc.x;
	//texel0.y = jumpc.y;

	vec4 diffel = texture2D(texture0, jumpc.xy);
	vec4 posxel = texture2D(posxtex, jumpc.xy);
	vec4 posyel = texture2D(posytex, jumpc.xy);
	vec4 poszel = texture2D(posztex, jumpc.xy);

	//texel0.xyz = posyel.xyz;
	//texel0 = texture2D(texture0, jumpc.xy);

	int steps = 5;
	int step = 0;

	vec4 posoff2;

	for(step=0; step<steps; ++step)
	{
		//diffel = vec4(jumpc.x,jumpc.y,0,1);
		vec4 posoff;
		posoff.x = (255 * posxel.x) + 256 * 255 * posxel.y - 30000;
		posoff.y = (255 * posyel.x) + 256 * 255 * posyel.y - 30000;
		posoff.z = (255 * poszel.x) + 256 * 255 * poszel.y - 30000;
		posoff.w = 1;

		posoff2 = mvp * posoff;
		posoff2 = posoff2 / posoff2.w;

		//texel0.x = posoff2.x;
		//texel0.y = posoff2.y;
		//texel0.z = posoff2.z;
	
		//float upoff = dot(updir, vec3(outpos2 - posoff2) );
		//float sideoff = dot(sidedir, vec3(outpos2 - posoff2) );

		//upoff = upoff / totuplen;
		//sideoff = sideoff / totsidelen;

		vec2 upnavc = jumpc + vec2(0, 1.0 / float(ormapsz));
		vec2 sidenavc = jumpc + vec2(1.0 / float(ormapsz), 0);
	
		vec4 posxelupnav = texture2D(posxtex, upnavc.xy);
		vec4 posyelupnav = texture2D(posytex, upnavc.xy);
		vec4 poszelupnav = texture2D(posztex, upnavc.xy);

		vec4 posxelsidenav = texture2D(posxtex, sidenavc.xy);
		vec4 posyelsidenav = texture2D(posytex, sidenavc.xy);
		vec4 poszelsidenav = texture2D(posztex, sidenavc.xy);

		vec4 upnavpos;
		upnavpos.x = (255 * posxelupnav.x) + 256 * 255 * posxelupnav.y - 30000;
		upnavpos.y = (255 * posyelupnav.x) + 256 * 255 * posyelupnav.y - 30000;
		upnavpos.z = (255 * poszelupnav.x) + 256 * 255 * poszelupnav.y - 30000;
		upnavpos.w = 1;

		vec4 sidenavpos;
		sidenavpos.x = (255 * posxelsidenav.x) + 256 * 255 * posxelsidenav.y - 30000;
		sidenavpos.y = (255 * posyelsidenav.x) + 256 * 255 * posyelsidenav.y - 30000;
		sidenavpos.z = (255 * poszelsidenav.x) + 256 * 255 * poszelsidenav.y - 30000;	
		sidenavpos.w = 1;

		//vec3 upnavpos = vec3(upnavposx,upnavposy,upnavposz);
		//vec3 sidenavpos = vec3(sidenavposx,sidenavposy,sidenavposz);

		upnavpos = mvp * upnavpos;
		sidenavpos = mvp * sidenavpos;

		//upnavpos = upnavpos / upnavpos.w;
		//sidenavpos = sidenavpos / sidenavpos.w;

		//texel0.x = upnavpos.x;
		//texel0.y = upnavpos.y;


		vec3 upnavpos2 = upnavpos.xyz;
		vec3 sidenavpos2 = sidenavpos.xyz;

		upnavpos2 = normalize( upnavpos2 );
		sidenavpos2 = normalize( sidenavpos2 );

		//component ratios
		//length?
		//vec2 offvec = vec2( sideoff, upoff );
		vec3 offvec = normalize( outpos2.xyz - posoff.xyz );
		float upcpt = dot( upnavpos2, offvec );
		float sidecpt = dot( sidenavpos2, offvec );

		jumpc = jumpc + vec2( upcpt, sidecpt ) * (steps - step) / float(ormapsz);
		//jumpc = jumpc + vec2( upcpt, sidecpt );

		diffel = texture2D(texture0, jumpc.xy);
		posxel = texture2D(posxtex, jumpc.xy);
		posyel = texture2D(posytex, jumpc.xy);
		poszel = texture2D(posztex, jumpc.xy);

		gl_FragColor.xyz = diffel.xyz;
		gl_FragColor.w = 1;
		gl_FragDepth = posoff2.z / posoff2.w;
	}

	
	if( length( posoff2.xyz - outpos2.xyz ) > 0.1)
		discard;

//empty space
//	if(jumpel.z > 0.99)
//		discard;
//wrong

	//vec4 texel0 = texture2D(texture0, gl_TexCoord[0].xy);
	//vec4 texel1 = texture(texture1, texCoordOut0);
	//vec4 texel2 = texture(texture2, texCoordOut0);
	//vec4 texel3 = texture(texture3, texCoordOut0);

	//texel0.xyz = diffel.xyz;

//	texel0 = vec4(viewdir.xyz, 1);

	//if(texel0.w <= 0.1)
	//	discard;

	//gl_FragColor = vec4(texel0.xyz, 1.0);
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

