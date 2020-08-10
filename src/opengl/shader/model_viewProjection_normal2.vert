R""(
layout(location = 0) uniform mat4 model;

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;

out vec3 _normal;

void main()
{
	gl_Position = model * vec4(position, 1.0);
	_normal = transpose(inverse(mat3(model))) * normal;
}
)""

