R""(
layout(location = 10, binding = 0) uniform sampler2D texture0;

in vec2 _texcoord;

out vec4 fragmentColor;

void main()
{
	fragmentColor = texture(texture0, _texcoord);
//	vec3 normal = texture(texture0, _texcoord);
//	fragmentColor = vec4(1.0, 0.0, 0.0, 1.0) + normal * 0.000001;//normal * 0.5 + 0.5;
}
)""
