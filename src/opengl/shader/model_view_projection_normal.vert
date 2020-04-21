R""(
layout(location = 0) uniform mat4 model;
layout(location = 4) uniform mat4 view;
layout(location = 8) uniform mat4 projection;

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;

out vec3 _position;
out vec3 _normal;

void main()
{
	vec4 modelPosition = model * vec4(position, 1.0);
	gl_Position = projection * (view * modelPosition);
	
	_position = vec3(modelPosition);
	_normal = transpose(inverse(mat3(model))) * normal;
}
)""

