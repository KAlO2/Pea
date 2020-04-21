R""(
//layout(std140) uniform Material
struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec4 specular_shininess;
};

//layout(std140) uniform PointLight
struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform PointLight light0;
layout(location = 14) uniform vec3 viewPosition;
//layout(location = 15) uniform vec3 lightPosition;
//layout(location = 16) uniform vec3 lightColor;

in vec3 _position;
in vec3 _normal;

out vec4 fragColor;

void main()
{
	vec3 normal = normalize(_normal);
	vec3 incident = normalize(_position - light0.position);
	// dot(normal, -incident) == -dot(normal, incident)
	float Ld = max(-dot(normal, incident), 0.0);
	
	vec3 specular = specular_shininess.xyz;
	float shininess = specular_shininess.w;
	
	vec3 viewDirection = normalize(viewPosition - _position);
	vec3 reflection = reflect(incident, normal);
	float Ls = pow(max(dot(viewDirection, reflection), 0.0), material.shininess);
	
	vec3 ambient = material.ambient * light0.ambient;
	vec3 diffuse = Ld * material.diffuse * light0.diffuse;
	vec3 specular= Ls * material.specular * light0.specular;
	fragColor = vec4(ambient + diffuse + specular, 1.0);
}
)""
