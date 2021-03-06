R""(
layout(location = 3) uniform vec3 ambientColor;
layout(location = 4) uniform vec3 objectColor;
layout(location = 5) uniform vec3 lightPosition;
layout(location = 7) uniform vec3 lightColor;
layout(location =11) uniform vec3 attenuation;

in vec3 _position;
in vec3 _normal;

out vec4 fragColor;

void main()
{
	vec3 lightDirection = lightPosition - _position;
	float distance = length(lightDirection);
	lightDirection /= distance;
	
	float Id = max(dot(_normal, lightDirection), 0.0);
	
	// attenuation_factor = 1 / (constant + linear * d + quadratic * d * d);
	float factor = 1 / dot(attenuation, vec3(1, distance, distance * distance));
	
	vec3 color = ambientColor + factor * Id * lightColor;
	color *= objectColor;
	fragColor = vec4(color, 1.0);
}
)""
