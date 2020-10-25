R""(
layout(location = 16, binding = 0) uniform sampler2D texture0;

in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	vec3 color = texture(texture0, _texcoord).rgb;
	fragColor = vec4(color, 1.0);
}
)""
