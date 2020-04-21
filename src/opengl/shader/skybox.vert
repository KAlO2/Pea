R""(
layout(location = 4) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;

out vec3 _texcoord;

void main()
{
	_texcoord = vec3(position.x, position.z, -position.y);
	vec4 position = viewProjection * vec4(position, 1.0);
	gl_Position = position.xyww;  // (x/w, y/w, 1.0)
}
)""
