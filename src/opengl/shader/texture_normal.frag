R""(
layout(location = 10, binding = 0) uniform sampler2D texture0;

in vec2 _texcoord;

out vec4 fragmentColor;

void main()
{
	vec3 normal = texture(texture0, _texcoord);
	fragmentColor = normal * 0.5 + 0.5;
}
)""
