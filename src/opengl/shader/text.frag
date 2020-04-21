R""(
uniform sampler2D texture;
uniform vec4 color;

void main()
{
	gl_FragColor = vec4(color.rgb, texture2D(texture, texcoord).r * color.a);
}
)""
