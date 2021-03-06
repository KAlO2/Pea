R""(
layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texcoord;

out vec3 _position;
out vec3 _normal;
out vec2 _texcoord;

void main()
{
	vec4 modelPosition = model * vec4(position, 1.0);
	gl_Position = viewProjection * modelPosition;
	
	_position = vec3(modelPosition);
	_normal = mat3(transpose(inverse(model))) * normal;
	
	_texcoord = texcoord;
}
)""
