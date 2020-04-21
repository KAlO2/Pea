R""(
layout(location = 16, binding = 0) uniform sampler2D texture0;
layout(location = 13) uniform vec2 radius_step;

in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	float radius = radius_step.x, step = radius_step.y;
	float N = floor(radius / step);
	vec3 color = vec3(0.0, 0.0, 0.0);
	if(N != 0.0)
	{
		for(int i = -N; i < N; ++i)
			color += texture(texture0, _texcoord + vec2(0.0, i * step)).rgb;
		
		color /= float(2 * N + 1);
	}
	else
		color = texture(texture0, _texcoord).rgb;
	fragColor = vec4(color, alpha);
}
)""
