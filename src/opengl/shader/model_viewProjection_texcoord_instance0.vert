R""(
layout(location = 1) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 texcoord;
layout(location = 6) in vec4 instance;

out vec2 _texcoord;

void main()
{
	mat4 model = mat4(
			instance.w, 0, 0, 0,
			0, instance.w, 0, 0,
			0, 0, instance.w, 0,
			instance.x, instance.y, instance.z, 1);
	
	gl_Position = viewProjection * (model * vec4(position, 1.0));
	_texcoord = texcoord;
}
)""
