R""(
uniform sampler3D texture0;

in vec3 _texcoord;

out vec4 fragColor;

void main()
{
	fragColor = texture(texture0, _texcoord);
}
)""
