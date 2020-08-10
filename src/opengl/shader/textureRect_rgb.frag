R""(
layout(location = 20) uniform sampler2DRect texture0;

in vec2 _texcoord;

out vec4 fragmentColor;

void main()
{
//	vec3 color = texture(texture0, _texcoord).rgb;
	vec3 color = texelFetch(texture0, ivec2(_texcoord)).rgb;
	fragmentColor = vec4(color, 1.0);
}
)""
