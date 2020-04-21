R""(
layout(location = 0) uniform mat4 model;
layout(location = 4) uniform mat4 view;
layout(location = 8) uniform mat4 projection;

layout (location = 0) in vec3 position;

void main()
{
	gl_Position = projection * view * (model * vec4(position, 1.0));
}
)""
