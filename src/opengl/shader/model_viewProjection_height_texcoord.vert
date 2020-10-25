R""(
layout(location = 0) uniform mat4 model;
layout(location = 1) uniform mat4 viewProjection;
layout(location =11) uniform vec3 scale;

uniform sampler2D textureHeight0;

layout(location = 0) in vec3 position;
layout(location = 3) in vec2 texcoord;

out vec3 _position;
out vec2 _texcoord;

void main()
{
	_texcoord = texcoord;
	float displacement = texture2DLod(textureHeight0, texcoord * scale.xy, 0.0).x;
//	float displacement = texture(textureHeight0, texcoord * scale.xy).x;
	vec4 vertex = vec4(position, 1.0);
	vertex.z += scale.z * displacement;
	gl_Position = viewProjection * (model * vertex);
}
)""
