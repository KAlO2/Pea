R""(
layout(location = 20, binding = 0) uniform sampler2D texture0;

uniform vec2 depthRange;

in vec2 _texcoord;

out vec4 fragColor;

float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	float near = depthRange.x, far = depthRange.y;
	return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
	float depth = texture(texture0, _texcoord).r;
	depth = LinearizeDepth(depth) / far;
	fragColor = vec4(depth, depth, depth, 1.0);
}
)""
