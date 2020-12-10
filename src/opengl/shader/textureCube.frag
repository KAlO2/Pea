R""(
layout(location =16) uniform samplerCube texture0;

in vec3 _texcoord;

out vec4 fragmentColor;

void main()
{
	fragmentColor = texture(texture0, _texcoord);
}
)""
