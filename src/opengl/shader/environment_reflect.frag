R""(
layout(location = 14) uniform vec3 viewPosition;
layout(location = 16) uniform samplerCube texture0;

in vec3 _position;
in vec3 _normal;

out vec4 fragColor;

void main()
{
	vec3 incident = normalize(_position - viewPosition);
	vec3 reflected = reflect(incident, _normal);
	vec3 texcoord = vec3(reflected.x, -reflected.z, reflected.y);
	vec3 color = texture(texture0, texcoord).rgb;
	fragColor = vec4(color, 1.0);
}
)""
