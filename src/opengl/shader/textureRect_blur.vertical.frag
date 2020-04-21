R""(
layout(location = 16, binding = 0) uniform sampler2DRect texture0;
layout(location = 13) uniform float radius;

in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	int r = round(radius);
	vec3 color =  vec3(0.0, 0.0, 0.0);
	for(int i = -r; i < +r; ++i)
		color += texture(texture0, texcoord + vec2(0.0, float(i)).rgb;
	color /= float(2 * r + 1);
	fragColor = vec4(color, alpha);
}
)""
