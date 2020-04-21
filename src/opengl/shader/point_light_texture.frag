R""(
struct Material
{
	vec3 ambient;
	sampler2D textureDiffuse;
	sampler2D textureSpecular;
	float shininess;
};

struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform vec3 viewPosition;
uniform Material material;
uniform PointLight light0;

in vec3 _position;
in vec3 _normal;
in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	// ambient
	vec3 ambient = light0.ambient * material.ambient;
	
	// diffuse
	vec3 normal = normalize(_normal);
	vec3 incident = normalize(_position - light0.position);
	// dot(normal, -incident) == -dot(normal, incident)
	float intensity = max(-dot(normal, incident), 0.0);
	vec3 materialDiffuse = texture(material.textureDiffuse, _texcoord).rgb;
	vec3 diffuse = light0.diffuse * materialDiffuse * intensity;
	
	// specular
	vec3 viewDirection = normalize(viewPosition - _position);
	vec3 reflection = reflect(incident, normal);
	intensity = pow(max(dot(viewDirection, reflection), 0.0), material.shininess);
	vec3 materialSpecular = texture(material.textureSpecular, _texcoord).rgb;
	vec3 specular = light0.specular * materialSpecular * intensity;
	
	vec3 color = ambient + diffuse + specular;
	fragColor = vec4(color, 1.0);
}
)""
