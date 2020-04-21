R""(
layout(location = 16, binding = 0) uniform sampler2D texture0;

in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	float luminance = texture(texture0, _texcoord).r;
	fragColor = vec4(luminance, luminance, luminance, 1.0);
}
)""
