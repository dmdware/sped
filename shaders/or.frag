

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

varying vec4 outpos;	//absolute coord

uniform int ormapsz;	//widths and heights of diffuse and position textures

uniform mat4 mvp;
uniform mat4 imvp;

void main (void)
{
vec4 texel0;

	vec4 outpos2 = outpos / outpos.w;
	outpos2 = imvp * outpos2;
	outpos2 = outpos2 / outpos2.w;


	vec3 updir = ( cornera.xyz - cornerd.xyz );
	vec3 sidedir = ( cornerc.xyz - cornerd.xyz );
	updir = normalize( updir );
	sidedir = normalize( sidedir );
	vec3 viewdir = - cross( sidedir, updir );
	viewdir = normalize( viewdir );


	float totuplen = length( (cornera.xyz - cornerd.xyz) );
	float totsidelen = length( (cornerc.xyz - cornerd.xyz) );

	float uprat = dot(updir, (outpos2.xyz - cornerd.xyz) / totsidelen );
	float siderat = dot(sidedir, (outpos2.xyz - cornerd.xyz) / totuplen );


	//jump table index coords

	int tabx = int(siderat*(orjplwpx-1)) + int(orjlon*orjplwpx*(orjlons-1));
	int taby = int(uprat*(orjplhpx-1)) + int(orjlat*orjplhpx*(orjlats-1));
	float tabxf = float(tabx)/(orjplwpx * orjlons);
	float tabyf = float(taby)/(orjplhpx * orjlats);

	vec2 startjump = vec2( tabxf, tabyf );
	vec4 jumpel = texture2D(jumptex, startjump);

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

		//(orlon,orlat) contains the rotation, 
		//so no need to apply rotation,
		//except to the rgb coords.

		//1 pixel offset up and right
		vec2 upnavc = jumpc + vec2(0, 1.0 / float(ormapsz));
		vec2 sidenavc = jumpc + vec2(1.0 / float(ormapsz), 0);
	
		vec4 posxelupnav = texture2D(posxtex, upnavc.xy);
		vec4 posyelupnav = texture2D(posytex, upnavc.xy);
		vec4 poszelupnav = texture2D(posztex, upnavc.xy);

		vec4 posxelsidenav = texture2D(posxtex, sidenavc.xy);
		vec4 posyelsidenav = texture2D(posytex, sidenavc.xy);
		vec4 poszelsidenav = texture2D(posztex, sidenavc.xy);

		//this is what space (offset) will be for 1 pixel 
		//move up on or. position/diffuse maps
		vec4 upnavpos;
		upnavpos.x = (255 * posxelupnav.x) + 256 * 255 * posxelupnav.y - 30000;
		upnavpos.y = (255 * posyelupnav.x) + 256 * 255 * posyelupnav.y - 30000;
		upnavpos.z = (255 * poszelupnav.x) + 256 * 255 * poszelupnav.y - 30000;
		upnavpos.w = 1;
		//get relative offset from current position in absolute space
		upnavpos = vec4( ( upnavpos.xyz -  posoff.xyz ), 1 );

		//this is what space (offset) will be for 1 pixel 
		//move right on or. position/diffuse maps
		vec4 sidenavpos;
		sidenavpos.x = (255 * posxelsidenav.x) + 256 * 255 * posxelsidenav.y - 30000;
		sidenavpos.y = (255 * posyelsidenav.x) + 256 * 255 * posyelsidenav.y - 30000;
		sidenavpos.z = (255 * poszelsidenav.x) + 256 * 255 * poszelsidenav.y - 30000;	
		sidenavpos.w = 1;
		//get relative offset from current position in absolute space
		sidenavpos = vec4( ( sidenavpos.xyz -  posoff.xyz ), 1 );


		//into-the-screen component
		float incpt;

		//eliminate into-the-screen direction offset
		incpt = dot( sidenavpos.xyz, viewdir );
		sidenavpos.xyz = sidenavpos.xyz - viewdir * incpt;
		sidenavpos.xyz = normalize( sidenavpos.xyz );
		//up
		incpt = dot( upnavpos.xyz, viewdir );
		upnavpos.xyz = sidenavpos.xyz - viewdir * incpt;
		upnavpos.xyz = normalize( upnavpos.xyz );


		//component ratios to get closer to the texture coords for diffuse.
		//offvec = needed offset of texture coords in absolute space to
		//the screen fragment.
		vec3 offvec = normalize( outpos.xyz - posoff.xyz );

		//eliminate into-the-screen direction offset
		incpt = dot( offvec, viewdir );
		offvec = offvec - viewdir * incpt;

		//normalize, because we want just the direction and have our
		//own jumping speed in pixels.
		offvec = normalize( offvec );

		//get component contributions of up,right along textures
		float upcpt = dot( upnavpos.xyz, offvec );
		float sidecpt = dot( sidenavpos.xyz, offvec );

		//jump a decreasing distance in pixels
		jumpc = jumpc + vec2( upcpt, sidecpt ) * (steps - step) / float(ormapsz);


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

