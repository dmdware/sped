

uniform lowp vec4 color;
uniform sampler2D texture0;

varying highp vec2 texcoordout0;

void main(void)
{
	lowp vec4 texel = texture2D(texture0, texcoordout0.xy);

	//if(texel.w <= 0.1)
	//	discard;

	texel = texel * color;
	//texel.w = 1.0;

	gl_FragColor = texel;
}
