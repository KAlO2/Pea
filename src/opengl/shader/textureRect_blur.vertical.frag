R""(
layout(location =16, binding = 0) uniform sampler2DRect texture0;
layout(location = 8) uniform int size;

in vec2 _texcoord;

layout(location = 1) out vec4 fragmentColor;

void main()
{
	vec3 color = vec3(0.0, 0.0, 0.0);
	for(int i = -size; i <= +size; ++i)
		color += texelFetch(texture0, ivec2(_texcoord) + ivec2(0, i)).rgb;
	
	color /= float(2 * size + 1);
	fragmentColor = vec4(color, 1.0);
}
)""
