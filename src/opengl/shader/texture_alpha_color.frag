R""(
layout(location = 10, binding = 0) uniform sampler2D texture0;

in vec2 texcoord;
flat in vec3 color;

out vec4 fragColor;

void main()
{
	float alpha = texture(texture0, texcoord).r;
	fragColor = vec4(color, alpha);
}
)""
