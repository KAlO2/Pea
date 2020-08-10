R""(
layout(location = 20, binding = 0) uniform sampler2D texture0;

in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	float depth = texture(texture0, _texcoord).r;
	fragColor = vec4(depth, depth, depth, 1.0);
}
)""
