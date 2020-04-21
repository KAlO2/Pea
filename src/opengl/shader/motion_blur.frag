R""(
layout(location = 16, binding = 0) uniform sampler2D texture0;

in vec3 _position;
in vec3 _position0;

out vec4 fragColor;

void main()
{
	const float sampleCount = 8;
	vec3 color = vec3(0.0, 0.0, 0.0);
	for(float i = 1; i <= sampleCount; ++i)
	{
		float w = i / sampleCount;
		vec3 position = lerp(_position0, _position, w);
		color += w * texture(texture0, position);
	}
	color /= sampleCount;
	
	fragColor = vec4(color, 1.0);
}
)""
