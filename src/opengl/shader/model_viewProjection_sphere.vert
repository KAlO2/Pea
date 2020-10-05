R""(
layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 viewProjection;

layout(location = 0) in vec3 position;

out vec2 _texcoord;

void main()
{
	gl_Position = viewProjection * (model * vec4(position, 1.0));
	
	const float PI = 3.14159265358979323846264338327950288;
	if(position.y != 0 && position.x != 0)
		_texcoord.s = atan(position.y, position.x) / (2 * PI) + 0.5;
	else
		_texcoord.s = position.z;
	
	_texcoord.t = asin(position.z) / PI + 0.5;
}
)""
