R""(
layout(location = 3) uniform vec4 textColor;
layout(location =10) uniform sampler2D texture0;

in vec2 texcoord;

out vec4 fragColor;

void main()
{
	float alpha = texture(texture0, texcoord).r;
	fragColor = vec4(textColor.rgb, textColor.a * alpha);
}
)""
