R""(
layout(location = 4) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;

void main()
{
	gl_Position = viewProjection * vec4(position, 1.0);
}
)""
