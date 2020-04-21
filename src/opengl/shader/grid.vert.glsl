R""(
layout(location = 0) uniform mat4 transform;

layout(location = 1) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec4 color;

out vec4 lineColor = color;

void main()
{
	gl_Position = transform * vec4(position, 1.0);
}
)""
