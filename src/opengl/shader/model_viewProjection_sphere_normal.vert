R""(
layout(location = 0) uniform mat4 model;
layout(location = 4) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;

out vec3 _position;
out vec3 _normal;

void main()
{
	vec4 modelPosition = model * vec4(position, 1.0);
	gl_Position = viewProjection * modelPosition;
	
	_position = modelPosition.xyz;
	
	vec3 normal = normalize(position);
	_normal = mat3(transpose(inverse(model))) * normal;
}
)""
