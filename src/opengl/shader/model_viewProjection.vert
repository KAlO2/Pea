R""(
layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;

out vec3 _position;

void main()
{
	_position = position;
	gl_Position = viewProjection * (model * vec4(position, 1.0));
}
)""
