R""(
layout(location =16, binding = 0) uniform sampler2D texture0;
layout(location =11) uniform float step;
layout(location =13) uniform int size;

in vec2 _texcoord;

layout(location = 0) out vec3 fragmentColor;

void main()
{
	vec3 color = vec3(0.0, 0.0, 0.0);
	for(int i = -size; i <= size; ++i)
		color += texture(texture0, _texcoord + vec2(0.0, i * step)).rgb;
	
	fragmentColor = color / float(2 * size + 1);
}
)""
