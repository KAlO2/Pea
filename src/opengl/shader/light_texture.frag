R""(
struct Material
{
	vec3 ambient;
	sample2D textureDiffuse;
	sample2D textureSpecular;
	float shininess;
};

struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform vec3 viewPosition;
uniform Material material;
uniform Light light;

in vec3 _position;
in vec3 _normal;
in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	// ambient
	vec3 ambient = light.ambient * material.ambient;
	
	// diffuse
	vec3 normal = normalize(_normal);
	vec3 incident = normalize(_position - light.position);
	// dot(normal, -incident) == -dot(normal, incident)
	float intensity = max(-dot(normal, incident), 0.0);
	vec3 materialDiffuse = texture(material.textureDiffuse, _texcoord).rgb;
	vec3 diffuse = light.diffuse * materialDiffuse * intensity;
	
	// specular
	vec3 viewDirection = normalize(viewPosition - _position);
	vec3 reflection = reflect(incident, normal);
	intensity = pow(max(dot(viewDirection, reflection), 0.0), material.shininess);
	vec3 materialSpecular = texture(material.textureSpecular, _texcoord).rgb;
	vec3 specular = light.specular * materialSpecular * intensity;
	
	vec3 color = ambient + diffuse + specular;
	fragColor = vec4(color, 1.0);
}
)""
