R""(
layout(location = 16, binding = 0) uniform sampler2D texture0;

in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	fragColor = texture(texture0, _texcoord);
}
)""
