R""(
layout(location = 0) uniform mat4 model;
layout(location = 4) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

out vec4 _color;

void main()
{
	gl_Position = viewProjection * (model * vec4(position, 1.0));
	_color = color;
}
)""
