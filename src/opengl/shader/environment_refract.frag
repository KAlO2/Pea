R""(
layout(location = 8) uniform vec3 viewPosition;
layout(location =11) uniform float ratio;
layout(location =16) uniform samplerCube texture0;

in vec3 _position;
in vec3 _normal;

out vec4 fragColor;

void main()
{
	// 1st infraction, into sphere, IOR = ratio;
	vec3 incident = normalize(_position - viewPosition);
	vec3 refracted = refract(incident, _normal, ratio);
	
	// 2nd infraction, out sphere, IOR = 1 / ratio;
	vec3 binormal = cross(incident, _normal);
	vec3 tangent = cross(_normal, binormal);
	// rotate 2 * theta;
	float theta = 2 * acos(dot(-_normal, refracted));
	vec3 normal = cos(theta) * _normal - sin(theta) * tangent;
	refracted = refract(refracted, normal, 1 / ratio);
	
	vec3 texcoord = vec3(refracted.x, -refracted.z, refracted.y);
	vec3 color = texture(texture0, texcoord).rgb;
	fragColor = vec4(color, 1.0);
}
)""
