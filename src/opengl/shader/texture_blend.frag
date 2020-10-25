R""(
layout(location = 16) uniform sampler2D texture0;
layout(location = 17) uniform sampler2D texture1;
layout(location = 11) uniform float weight;

in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	vec4 color0 = texture(texture0, _texcoord);
	vec4 color1 = texture(texture1, _texcoord);
	fragColor = mix(color0, color1, weight);
}
)""
