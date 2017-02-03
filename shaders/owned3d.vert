
#version 120

attribute vec4 position;

uniform mat4 projection;
uniform mat4 model;
uniform mat4 view;
uniform mat4 normalMatrix;

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
uniform mat4 normalMat;

void main(void)
{
	//vec4 vpos = (view * (model * position));
	vec4 vpos = model * gl_Vertex;
	//vec4 vpos = position;
	//vpos.w = 1;	//ortho=1/persp?
	lpos = lightMatrix * vpos;
	//lpos.w = 1;
	gl_Position = projection * (view * (model * gl_Vertex));
	//gl_Position.w = 1;	//ortho=1/persp?

	light_vec = vpos.xyz - lightPos;

	//vec3 normalEyeSpace = vec3( normalMatrix * vec4(normalIn, 0.0) );
	//vec3 normalEyeSpace = mat3(normalMatrix) * normalIn;
	//mat4 normalMat = transpose( inverse( model * view ) );
	//mat4 normalMat = invModelView;
	vec3 normalEyeSpace = vec3( normalMat * vec4(gl_Normal, 0.0) );
	normalOut = normalize(normalEyeSpace);

	texCoordOut0 = texCoordIn0;
}
