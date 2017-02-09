
#version 120

attribute vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 normalMatrix;
uniform mat4 mvp;

uniform mat4 lightMatrix;
uniform vec3 lightPos;
uniform vec3 lightDir;

varying vec4 lpos;
varying vec3 light_vec;
varying vec3 light_dir;

attribute vec3 normalIn;
varying vec3 normalOut;

attribute vec2 texCoordIn0;
varying vec2 texCoordOut0;

//uniform mat4 invModelView;
//uniform mat4 normalMat;

varying vec3 eyevec;
//attribute vec3 tangent;

uniform float maxelev;
//varying float elevtransp;
varying float elevy;

uniform vec3 sundirection;
uniform mat4 normalMat;


varying vec4 outpos;


	/*
	The difference from ortho here,
	is the "viewdir" must be defined for
	each screen fragment.
	*/

varying vec3 perspview;
uniform vec3 camcen;

void main(void)
{
	//vec4 vpos = (view * (model * position));
	vec4 vpos = model * gl_Vertex;
	//vec4 vpos = position;
	//vpos.w = 1;	//ortho=1/persp?
	lpos = lightMatrix * vpos;
	//lpos.w = 1;
	//gl_Position = projection * view * model * gl_Vertex;
	gl_Position = mvp * gl_Vertex;
	//gl_Position.w = 1;	//ortho=1/persp?

	perspview = normalize( gl_Vertex.xyz - camcen );

	//IMPORTANT:
	//the gl_Vertex coordinates must be in absolute space.
	//any translation must be added to the vertices before
	//sending them here.
	//rotations must be defined in (orjlon,orjlat) relative
	//to the layed out structure of corners of the quad
	//which is already perpindicular to the viewing angle.
	//if a model transformation is applied,
	//outpos2 in the fragment shader must be the absolute
	//space coordinate after all the translation and rotation.

	//absolute
	outpos = gl_Vertex;

	elevy = gl_Vertex.y;
	//elevtransp = 1;

	//if(position.y > maxelev)
	//{
	//	elevtransp = 0;
	//}

	vpos = (view * (model * gl_Vertex));

	//vec3 normalEyeSpace = vec3( normalMatrix * vec4(normalIn, 0.0) );
	//vec3 normalEyeSpace = mat3(normalMatrix) * normalIn;
	//mat4 normalMat = transpose( inverse( model * view ) );
	//mat4 normalMat = transpose( inverse( model ) );
	//mat4 normalMat = invModelView;
	vec3 normalEyeSpace = vec3( normalMat * vec4(gl_Normal, 0.0) );
	normalOut = normalize(normalEyeSpace);

	vec3 n = normalOut;
	//vec3 tangentEyeSpace = vec3( normalMat * vec4(tangent, 0.0) );
	//vec3 t = normalize(tangentEyeSpace);
	//vec3 t = normalOut;

	vec3 t;
	vec3 c1 = cross( normalOut, vec3(0.0, 0.0, 1.0) ); 
	vec3 c2 = cross( normalOut, vec3(0.0, 1.0, 0.0) ); 

	if( length(c1)>length(c2) )
	{
		t = normalize(c1);	
	}
	else
	{
		t = normalize(c2);	
	}

	vec3 b = normalize(cross(n, t));
	//vec3 b = normalOut;

	vec3 vVertex = vec3(view * model * gl_Vertex);

	//light_vec = vpos.xyz - lightPos;
	//vec3 tmpVec = lightPos - vVertex;
	vec3 tmpVec = sundirection;
	light_vec.x = dot(tmpVec, t);
	light_vec.y = dot(tmpVec, b);
	light_vec.z = dot(tmpVec, n);

	//light_vec = n;
	//light_vec = normalIn * 0.5 + 0.5;
	//light_vec = t;
	//light_vec = t * 0.5 + 0.5;
	//light_vec = b * 0.5 + 0.5;

	tmpVec = -vVertex;
	eyevec.x = dot(tmpVec, t);
	eyevec.y = dot(tmpVec, b);
	eyevec.z = dot(tmpVec, n);

	gl_TexCoord[0] = gl_MultiTexCoord0;
}
