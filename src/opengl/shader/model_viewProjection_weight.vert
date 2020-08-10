R""(
layout(location = 0) uniform mat4 model;
layout(location = 4) uniform mat4 viewProjection;

layout(location =20, binding = 0) uniform sampler1D texture0;  // color ramp

layout(location = 0) in vec3 position;
layout(location = 7) in float weight;

out vec4 _color;

void main()
{
	gl_Position = viewProjection * (model * vec4(position, 1.0));
	_color = texture1D(texture0, weight);
}
)""
