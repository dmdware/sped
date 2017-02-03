#version 120
#extension GL_EXT_gpu_shader4 : enable

uniform float width;
uniform float height;
uniform float screenmapwidth;
uniform float screenmapheight;

void main(void)
{
   // gl_Position = vec4(gl_Vertex.x * 2.0 / width - 1.0,
    //                   gl_Vertex.y * -2.0 / height + 1.0,
    //                   0,
     //                  1.0);

	float revx = gl_Vertex.x;
	float revy = height - gl_Vertex.y;

	//revy = screen	


    gl_Position = vec4(revx * 2.0 / width - 1.0,
                       revy * -2.0 / height + 1.0,
                       0,
                       1.0);

	//vec2 onscreenratio = vec2( gl_Vertex.x / width, gl_Vertex.y / height ); 
	//vec2 onbufferratio = vec2( onscreenratio.x * width / screenmapwidth, (onscreenratio.y) * height / screenmapheight );
	//vec2 rangedratio = vec2( onbufferratio.x * 2.0 - 1.0, onbufferratio.y * -2.0 + 1.0 );

	//vec2 onscreenratio = vec2( gl_Vertex.x / width, gl_Vertex.y / height ); 
	//vec2 onbufferratio = vec2( onscreenratio.x * width / screenmapwidth, (1.0 - onscreenratio.y) * height / screenmapheight );
	//vec2 rangedratio = vec2( onbufferratio.x * 2.0 - 1.0, onbufferratio.y * -2.0 + 1.0 );

	//vec2 onscreenratio = vec2( gl_Vertex.x / width, gl_Vertex.y / height ); 
	//vec2 onbufferratio = vec2( onscreenratio.x * width / screenmapwidth, (1.0 - onscreenratio.y) * height / screenmapheight );
	//vec2 rangedratio = vec2( onbufferratio.x * 2.0 - 1.0, onbufferratio.y * 2.0 - 1.0 );

	//vec2 onscreenratio = vec2( gl_Vertex.x / width, (height - gl_Vertex.y) / height ); 
	//vec2 onbufferratio = vec2( onscreenratio.x * width / screenmapwidth, (onscreenratio.y) * height / screenmapheight );
	//vec2 rangedratio = vec2( onbufferratio.x * 2.0 - 1.0, onbufferratio.y * 2.0 - 1.0 );

   // gl_Position = vec4(rangedratio.x,
    //                   rangedratio.y,
    //                   0,
   //                    1.0);
    
    gl_TexCoord[0] = gl_MultiTexCoord0;

    //gl_TexCoord[0] = vec4( gl_MultiTexCoord0.x, 1.0 - gl_MultiTexCoord0.y, 
	//		gl_MultiTexCoord0.z, gl_MultiTexCoord0.w );
}