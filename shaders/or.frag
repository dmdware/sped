

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

//decode binary
vec4 DecBin(vec4 posxel, vec4 posyel, vec4 poszel)
{
	vec4 pos;

	pos.x = (255 * posxel.x) + 256 * 255 * posxel.y - 30000;
	pos.y = (255 * posyel.x) + 256 * 255 * posyel.y - 30000;
	pos.z = (255 * poszel.x) + 256 * 255 * poszel.y - 30000;
	pos.w = 1;

	return pos;
}

//decompose vector into two components.
//ignore into-the-screen direction.
//up and side are assumed to be normalized.
//return the resulting vector.
vec3 Decompose(vec3 v, vec3 side, vec3 up)
{	
	//components
	vec2 cpts;

	cpts.x = dot(v, side);
	v = v - side * cpts.x;
	cpts.y = dot(v, up);

	return ( side * cpts.x + up * cpts.y );
}

//also decompose, like above, but get the ratios
//for the components, instead of resulting vector.
vec2 Decompose2(vec3 v, vec3 side, vec3 up)
{
	//components
	vec2 cpts;

	cpts.x = dot(v, side);
	v = v - side * cpts.x;
	cpts.y = dot(v, up);

	return cpts;
}

void main (void)
{
vec4 texel0;

	vec3 cen = ( cornera.xyz + cornerc.xyz ) / 2.0;

	//vec4 outpos2 = outpos / outpos.w;
	//outpos2 = imvp * outpos2;
	//outpos2 = outpos2 / outpos2.w;
	vec4 outpos2 = outpos;
	//vec4 outpos2 = gl_FragCoord;
	//outpos2.w = 1;
	//outpos2 = mvp * outpos;
	//outpos2 = outpos2 / outpos2.w;


	//in image, top-left is origin.
	//in SpEd, space coords going up is increasing y coord value,
	//which means updir in space is reverse of texture y coord increase.
	//so use -1 for movement up along texture
	vec3 updir = ( cornera.xyz - cornerd.xyz );
	vec3 sidedir = ( cornerc.xyz - cornerd.xyz );
	float totuplen = length( updir );
	float totsidelen = length( sidedir );
	updir = ( updir ) / totuplen;
	sidedir = ( sidedir ) / totsidelen;
	vec3 viewdir = - cross( sidedir, updir );
	viewdir = normalize( viewdir );


	//uprat is measured from the bottom space coord going up,
	//but gives negative pixel image y coord going down,
	//so make it within [0..1]. EDIT: will use -1 tex jump instead.
	float uprat = dot(updir, (outpos2.xyz - cornerd.xyz) / totsidelen );
	float siderat = dot(sidedir, (outpos2.xyz - cornerd.xyz) / totuplen );

	//uprat = 1.0 - uprat;


	//outpos2.x = siderat;
	//outpos2.y = uprat;
	//outpos2.z = 0;

	//outpos2 = texture2D(texture0, outpos2.xy);


	//eliminate into-the-screen direction offset
	//float incpt;
	//incpt = dot(outpos2.xyz, viewdir);
	//outpos2.xyz = outpos2.xyz - viewdir * incpt;


	//jump table index coords

	int tabx = int(siderat*(orjplwpx-1)) + int(orjlon*(orjlons-1)*orjplwpx);
	int taby = int(uprat*(orjplhpx-1)) + int(orjlat*(orjlats-1)*orjplhpx);
	//int taby = int((1.0-uprat)*(orjplhpx-1)) + int(orjlat*(orjlats-1)*orjplhpx);
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

	//if(jumpi == 0)
	//	discard;

	vec2 jumpc = vec2( 
			float( (jumpi%ormapsz) ) / float(ormapsz), 
			float( (jumpi/ormapsz) ) / float(ormapsz) );

	if(jumpi == 0)
	{
		jumpc.x = 0.5;
		jumpc.y = 0.5;
	}

	vec4 diffel = texture2D(texture0, jumpc.xy);
	vec4 posxel = texture2D(posxtex, jumpc.xy);
	vec4 posyel = texture2D(posytex, jumpc.xy);
	vec4 poszel = texture2D(posztex, jumpc.xy);

	int steps = 70;
	int step = 0;

	vec4 offpos;

	for(step=0; step<steps; ++step)
	{
		//diffel = vec4(jumpc.x,jumpc.y,0,1);
		offpos = DecBin(posxel, posyel, poszel);
		//rotate based on viewing angle
		offpos.xyz = RotAr(offpos.xyz, cen, latrad, 1, 0, 0);
		offpos.xyz = RotAr(offpos.xyz, cen, lonrad, 0, 1, 0);

		float pixjump = float(steps - step);


		//(orlon,orlat) contains the rotation.
		//apply rotation to the rgb coords.

		//1 pixel offset up and right
		vec2 upnavc = jumpc + vec2(0, -pixjump / float(ormapsz));
		vec2 sidenavc = jumpc + vec2(pixjump / float(ormapsz), 0);
	
		posxel = texture2D(posxtex, upnavc.xy);
		posyel = texture2D(posytex, upnavc.xy);
		poszel = texture2D(posztex, upnavc.xy);
		//this is what space (offset) direction will be for 1 pixel 
		//move up on or. position/diffuse maps.
		vec4 upnavoff = DecBin(posxel, posyel, poszel);
		//rotate based on viewing angle
		upnavoff.xyz = RotAr(upnavoff.xyz, cen, latrad, 1, 0, 0);
		upnavoff.xyz = RotAr(upnavoff.xyz, cen, lonrad, 0, 1, 0);
		//get relative offset from current position in absolute space
		upnavoff = vec4( ( upnavoff.xyz -  offpos.xyz ), 1 );

		posxel = texture2D(posxtex, sidenavc.xy);
		posyel = texture2D(posytex, sidenavc.xy);
		poszel = texture2D(posztex, sidenavc.xy);
		//this is what space (offset) direction will be for 1 pixel 
		//move right on or. position/diffuse maps.
		vec4 sidenavoff = DecBin(posxel, posyel, poszel);
		//rotate based on viewing angle
		sidenavoff.xyz = RotAr(sidenavoff.xyz, cen, latrad, 1, 0, 0);
		sidenavoff.xyz = RotAr(sidenavoff.xyz, cen, lonrad, 0, 1, 0);
		//get relative offset from current position in absolute space
		sidenavoff = vec4( ( sidenavoff.xyz -  offpos.xyz ), 1 );


		//eliminate into-the-screen direction offset
		//side
		sidenavoff.xyz = Decompose(sidenavoff.xyz, sidedir, updir);
		sidenavoff.xyz = normalize( sidenavoff.xyz );
		//up
		upnavoff.xyz = Decompose(upnavoff.xyz, sidedir, updir);
		upnavoff.xyz = normalize( upnavoff.xyz );


		//component ratios to get closer to the texture coords for diffuse.
		//offvec = needed offset of texture coords in absolute space to
		//the screen fragment.
		vec3 offvec = ( outpos2.xyz - offpos.xyz );
		offvec = Decompose(offvec, sidedir, updir);


		//normalize, because we want just the direction and have our
		//own jumping speed in pixels.
		offvec = normalize( offvec );

		//get component contributions of up,right along textures
		vec2 texjump = Decompose2( offvec, sidenavoff.xyz, upnavoff.xyz );
		texjump.y = - texjump.y;
		texjump = normalize( texjump );


		//jump a decreasing distance in pixels
		jumpc = jumpc + texjump * pixjump / float(ormapsz);

		//jumpc = jumpc + texjump / float(ormapsz);


		diffel = texture2D(texture0, jumpc.xy);
		posxel = texture2D(posxtex, jumpc.xy);
		posyel = texture2D(posytex, jumpc.xy);
		poszel = texture2D(posztex, jumpc.xy);

		gl_FragColor.xyz = diffel.xyz;
		gl_FragColor.w = 1;

		offpos = DecBin(posxel, posyel, poszel);
		//rotate based on viewing angle
		offpos.xyz = RotAr(offpos.xyz, cen, latrad, 1, 0, 0);
		offpos.xyz = RotAr(offpos.xyz, cen, lonrad, 0, 1, 0);


		vec4 mvppos = mvp * offpos;
		//mvppos = mvppos / mvppos.w;
		gl_FragDepth = mvppos.z / mvppos.w;
	}

	if( length( offpos.xyz - outpos2.xyz ) > 80 )
		discard;

	//gl_FragColor.xyz = outpos2.xyz;
	//gl_FragColor.w = 1;

	//gl_FragDepth = 0.1;

	
	//if( length( offpos.xyz - outpos2.xyz ) > 10000)
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

