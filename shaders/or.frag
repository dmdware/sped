

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

uniform float orjlon;	//ratio [0..1]
uniform float orjlat;

varying vec4 outpos;
uniform int ormapsz;	//widths and heights of diffuse and position textures

uniform mat4 mvp;
uniform mat4 imvp;

vec3 Rot(vec3 v, float rad, float x, float y, float z)
{
	vec3 newV;
	float cosTheta = cos(rad);
	float sinTheta = sin(rad);

	newV.x  = (cosTheta + (1 - cosTheta) * x * x)		* v.x;
	newV.x += ((1 - cosTheta) * x * y - z * sinTheta)	* v.y;
	newV.x += ((1 - cosTheta) * x * z + y * sinTheta)	* v.z;

	newV.y  = ((1 - cosTheta) * x * y + z * sinTheta)	* v.x;
	newV.y += (cosTheta + (1 - cosTheta) * y * y)		* v.y;
	newV.y += ((1 - cosTheta) * y * z - x * sinTheta)	* v.z;

	newV.z  = ((1 - cosTheta) * x * z - y * sinTheta)	* v.x;
	newV.z += ((1 - cosTheta) * y * z + x * sinTheta)	* v.y;
	newV.z += (cosTheta + (1 - cosTheta) * z * z)		* v.z;

	return newV;
}

vec3 RotAr(vec3 v, vec3 around, float rad, float x, float y, float z)
{
	v = v - around;
	v = Rot(v, rad, x, y, z);
	v = v + around;
	return v;	
}

vec4 DecBin(vec3 posxel, vec3 posyel, vec3 poszel)
{
	vec4 pos;

	pos.x = (255 * posxel.x) + 256 * 255 * posxel.y - 30000;
	pos.y = (255 * posyel.x) + 256 * 255 * posyel.y - 30000;
	pos.z = (255 * poszel.x) + 256 * 255 * poszel.y - 30000;
	pos.w = 1;

	return pos;
}

void main (void)
{
vec4 texel0;

	vec3 cen = ( cornera.xyz + cornerc.xyz ) / 2.0;

	vec4 outpos2 = outpos / outpos.w;
	outpos2 = imvp * outpos2;
	outpos2 = outpos2 / outpos2.w;


	vec3 updir = ( cornera.xyz - cornerd.xyz );
	vec3 sidedir = ( cornerc.xyz - cornerd.xyz );
	float totuplen = length( updir );
	float totsidelen = length( sidedir );
	updir = ( updir ) / totuplen;
	sidedir = ( sidedir ) / totsidelen;
	vec3 viewdir = - cross( sidedir, updir );
	viewdir = normalize( viewdir );


	float uprat = dot(updir, (outpos2.xyz - cornerd.xyz) / totsidelen );
	float siderat = dot(sidedir, (outpos2.xyz - cornerd.xyz) / totuplen );


	//eliminate into-the-screen direction offset
	//float incpt;
	//incpt = dot(outpos2.xyz, viewdir);
	//outpos2.xyz = outpos2.xyz - viewdir * incpt;


	//jump table index coords

	int tabx = int(siderat*(orjplwpx-1)) + int(orjlon*orjplwpx*(orjlons-1));
	int taby = int(uprat*(orjplhpx-1)) + int(orjlat*orjplhpx*(orjlats-1));
	float tabxf = float(tabx)/(orjplwpx * orjlons);
	float tabyf = float(taby)/(orjplhpx * orjlats);

	//the angle of viewing of the object
	float lonrad = 3.1415 * 2.0 * orjlon;
	float latrad = 3.1415 * 1.0 * orjlat;

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

	int steps = 700;
	int step = 0;

	vec4 posoff;

	for(step=0; step<steps; ++step)
	{
		//diffel = vec4(jumpc.x,jumpc.y,0,1);
		posoff = DecBin(posxel, posyel, poszel);
		//rotate based on viewing angle
		//posoff.xyz = RotAr(posoff.xyz, cen, latrad, 1, 0, 0);
		//posoff.xyz = RotAr(posoff.xyz, cen, lonrad, 0, 1, 0);


		//(orlon,orlat) contains the rotation.
		//apply rotation to the rgb coords.

		//1 pixel offset up and right
		vec2 upnavc = jumpc + vec2(0, 1.0 / float(ormapsz));
		vec2 sidenavc = jumpc + vec2(1.0 / float(ormapsz), 0);
	
		posxel = texture2D(posxtex, upnavc.xy);
		posyel = texture2D(posytex, upnavc.xy);
		poszel = texture2D(posztex, upnavc.xy);
		//this is what space (offset) direction will be for 1 pixel 
		//move up on or. position/diffuse maps.
		vec4 upnavpos = DecBin(posxel, posyel, poszel);
		//rotate based on viewing angle
		//upnavpos.xyz = RotAr(upnavpos.xyz, cen, latrad, 1, 0, 0);
		//upnavpos.xyz = RotAr(upnavpos.xyz, cen, lonrad, 0, 1, 0);
		//get relative offset from current position in absolute space
		upnavpos = vec4( ( upnavpos.xyz -  posoff.xyz ), 1 );

		posxel = texture2D(posxtex, sidenavc.xy);
		posyel = texture2D(posytex, sidenavc.xy);
		poszel = texture2D(posztex, sidenavc.xy);
		//this is what space (offset) direction will be for 1 pixel 
		//move right on or. position/diffuse maps.
		vec4 sidenavpos = DecBin(posxel, posyel, poszel);
		//rotate based on viewing angle
		//sidenavpos.xyz = RotAr(sidenavpos.xyz, cen, latrad, 1, 0, 0);
		//sidenavpos.xyz = RotAr(sidenavpos.xyz, cen, lonrad, 0, 1, 0);
		//get relative offset from current position in absolute space
		sidenavpos = vec4( ( sidenavpos.xyz -  posoff.xyz ), 1 );


		//eliminate into-the-screen direction offset
		//side
		uprat = dot(updir, (sidenavpos.xyz)  );
		siderat = dot(sidedir, (sidenavpos.xyz) );
		sidenavpos.xyz = updir * uprat + sidedir * siderat;
		sidenavpos.xyz = normalize( sidenavpos.xyz );
		//up
		uprat = dot(updir, (upnavpos.xyz)  );
		siderat = dot(sidedir, (upnavpos.xyz) );
		upnavpos.xyz = updir * uprat + sidedir * siderat;
		upnavpos.xyz = normalize( upnavpos.xyz );


		//component ratios to get closer to the texture coords for diffuse.
		//offvec = needed offset of texture coords in absolute space to
		//the screen fragment.
		vec3 offvec = ( outpos2.xyz - posoff.xyz );

		//eliminate into-the-screen direction offset
		uprat = dot(updir, (offvec)  );
		siderat = dot(sidedir, (offvec) );
		offvec = updir * uprat + sidedir * siderat;

		//normalize, because we want just the direction and have our
		//own jumping speed in pixels.
		offvec = normalize( offvec );

		//get component contributions of up,right along textures
		float upcpt = dot( upnavpos.xyz, offvec );
		offvec = offvec - upnavpos.xyz * upcpt;
		float sidecpt = dot( sidenavpos.xyz, offvec );

		//jump a decreasing distance in pixels
		//jumpc = jumpc + vec2( sidecpt, upcpt ) * (steps - step) / float(ormapsz);

		jumpc = jumpc + vec2( sidecpt, upcpt ) / float(ormapsz);


		diffel = texture2D(texture0, jumpc.xy);
		posxel = texture2D(posxtex, jumpc.xy);
		posyel = texture2D(posytex, jumpc.xy);
		poszel = texture2D(posztex, jumpc.xy);

		gl_FragColor.xyz = diffel.xyz;
		gl_FragColor.w = 1;

		posoff.x = (255 * posxel.x) + 256 * 255 * posxel.y - 30000;
		posoff.y = (255 * posyel.x) + 256 * 255 * posyel.y - 30000;
		posoff.z = (255 * poszel.x) + 256 * 255 * poszel.y - 30000;
		posoff.w = 1;
		//rotate based on viewing angle
		//posoff.xyz = RotAr(posoff.xyz, cen, latrad, 1, 0, 0);
		//posoff.xyz = RotAr(posoff.xyz, cen, lonrad, 0, 1, 0);


		vec4 mvppos = mvp * posoff;
		//mvppos = mvppos / mvppos.w;
		gl_FragDepth = mvppos.z / mvppos.w;
	}

	
	//if( length( posoff.xyz - outpos2.xyz ) > 10000)
	//	discard;

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

