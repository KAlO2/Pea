R""(
layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 model0;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 7) in vec2 size;

layout(location = 0) in vec3 position0;

out vec3 _position;
out vec3 _position0;

void main()
{
	// transform previous and current position to eye space
	vec4 _position = view * (model * vec4(position, 1.0));
	vec4 _position0 = view * (model0 * vec4(position0, 1.0));
	
	gl_Position = _position;
}
)""
