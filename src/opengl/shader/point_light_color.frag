R""(
//layout(std140) uniform material
struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec4 specular_shininess;
};

uniform Material material;

layout(location = 4) uniform vec3 viewPosition;
layout(location = 5) uniform vec3 lightPosition;
layout(location = 7) uniform vec3 lightColor;

in vec3 _position;
in vec3 _normal;

out vec4 fragColor;

void main()
{
	vec3 normal = normalize(_normal);
	vec3 incident = normalize(_position - lightPosition);
	// dot(normal, -incident) == -dot(normal, incident)
	float Ld = max(-dot(normal, incident), 0.0);
	
	vec3 specular = material.specular_shininess.xyz;
	float shininess = material.specular_shininess.w;
	
	vec3 viewDirection = normalize(viewPosition - _position);
	vec3 reflection = reflect(incident, normal);
	float Ls = pow(max(dot(viewDirection, reflection), 0.0), shininess);
	
	vec3 materialColor = material.ambient + Ld * material.diffuse + Ls * specular;
	fragColor = vec4(materialColor * lightColor, 1.0);
}
)""
