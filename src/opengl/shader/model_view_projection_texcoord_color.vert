R""(
layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 view;
layout(location = 2) uniform mat4 projection;

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 texcoord;

out vec2 _texcoord;
???
void main()
{
	gl_Position = projection * (view * (model * vec4(position, 1.0)));
	_texcoord = texcoord;
}
)""
