R""(
const int COUNT = 7;

layout(location = 10, binding = 0) uniform sampler2D texture[COUNT];

in vec2 texcoord;

out vec4 fragColor;

void main()
{
	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);
	for(int i = 0; i < COUNT; ++i)
		color += texture(texture[i], texcoord);
	
	fragColor = vec4(color.rgb / COUNT, 1.0);
}
)""
