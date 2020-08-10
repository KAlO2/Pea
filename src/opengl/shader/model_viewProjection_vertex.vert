R""(
layout(location = 0) uniform mat4 model;
layout(location = 4) uniform mat4 viewProjection;

layout(binding = 0) uniform sampler1D texture0;  // color ramp

layout(location = 0) in vec4 vertex;  // vec3 position; float weight;


out vec4 _color;

void main()
{
	gl_Position = viewProjection * (model * vec4(vertex.xyz, 1.0));
	_color = texture1D(texture0, vertex.w);
}
)""
