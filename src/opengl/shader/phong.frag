R""(
layout(location = 3) uniform vec3 ambientColor;
layout(location = 4) uniform vec3 objectColor;
layout(location = 5) uniform vec3 lightPosition;
layout(location = 7) uniform vec3 lightColor;
layout(location = 8) uniform vec3 viewPosition;
layout(location =10) uniform vec4 specularColor_shininess;
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
	
	vec3 specularColor = specularColor_shininess.xyz;
	float shininess = specularColor_shininess.w;
	vec3 reflected = reflect(-lightDirection, _normal);
	vec3 viewDirection = normalize(viewPosition - _position);
	float Is = pow(max(dot(reflected, viewDirection), 0.0, shininess);
	
	// attenuation_factor = 1 / (constant + linear * d + quadratic * d * d);
	float factor = 1 / dot(attenuation, vec3(1, distance, distance * distance));
	
	vec3 color = ambientColor + factor * (Id * lightColor + Is * specularColor);
	color *= objectColor;
	fragColor = vec4(color, 1.0);
}
)""
