R""(
layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 viewProjection;
layout(location = 3) uniform float radius;

layout(location = 0) in vec3 position;

out vec3 _normal;

void main()
{
	gl_Position = viewProjection * (model * vec4(position, 1.0));
	
	_normal = position / radius;
}
)""

