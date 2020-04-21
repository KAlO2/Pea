R""(
layout(location = 14) uniform vec3 viewPosition;
layout(location = 16) uniform samplerCube texture0;

uniform float ratio;

in vec3 _position;
in vec3 _normal;

out vec4 fragColor;

void main()
{
	vec3 incident = normalize(_position - viewPosition);
	vec3 refracted = refract(incident, _normal, ratio);
	vec3 color = texture(texture0, reflected).rgb;
	fragColor = vec4(color, 1.0);
}
)""
