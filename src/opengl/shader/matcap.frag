R""(
layout(location = 10, binding = 0) uniform sampler2D texture0;

in vec3 _normal;

out vec4 fragColor;

void main()
{
	vec2 _texcoord = _normal.xy * 0.5 + 0.5;
	vec3 color = texture(texture0, _texcoord).rgb;
	fragColor = vec4(color, 1.0);
}
)""
