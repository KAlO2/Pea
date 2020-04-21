R""(
layout(location = 8) uniform mat3 projection;

layout(location = 0) in vec4 vertex;

out vec2 texcoord;

void main()
{
	vec3 position = projection * vec3(vertex.xy, 1.0);
//	gl_Position.xyw = position;
	gl_Position = vec4(position.x, position.y, 0, position.z);
	texcoord = vertex.zw;
}
)""
