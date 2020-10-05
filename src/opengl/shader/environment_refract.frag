R""(
layout(location = 4) uniform vec3 viewPosition;
layout(location = 7) uniform float ratio;
layout(location =10) uniform samplerCube texture0;

in vec3 _position;
in vec3 _normal;

out vec4 fragColor;

void main()
{
	vec3 incident = normalize(_position - viewPosition);
	vec3 refracted = refract(incident, _normal, ratio);
	vec3 texcoord = vec3(refracted.x, -refracted.z, refracted.y);
	vec3 color = texture(texture0, texcoord).rgb;
	fragColor = vec4(color, 1.0);
}
)""
