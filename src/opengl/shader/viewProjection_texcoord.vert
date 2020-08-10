R""(
layout(location = 4) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 texcoord;

out vec2 _texcoord;

void main()
{
	gl_Position = viewProjection * vec4(position, 1.0));
	_texcoord = texcoord;
}
)""
