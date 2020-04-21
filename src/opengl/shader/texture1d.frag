R""(
uniform sampler1D texture0;

in float _texcoord;

void main()
{
	gl_FragColor = texture(texture0, _texcoord);
}
)""
