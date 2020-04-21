R""(
layout(binding = 0) uniform sampler2D textureDiffuse0;
layout(binding = 1) uniform sampler2D textureSpecular0;
layout(binding = 2) uniform sampler2D textureNormal0;

in vec3 _lightPosition;
in vec3 _viewPosition;
in vec3 _position;
in vec3 _normal;
in vec2 _texcoord;

out vec4 fragColor;

void main()
{
	// obtain normal from normal map in range [0,1]
	vec3 normal = texture(textureNormal0, _texcoord).rgb;
	// transform normal vector to range [-1,1]
	normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space

	// get diffuse color
	vec3 diffuseColor = texture(textureDiffuse0, _texcoord).rgb;
	vec3 specularColor = texture(textureSpecular0, _texcoord).rgb;
	float Ia = 0.1;
	float shininess = 10.0;
	
	vec3 lightDirection = normalize(_lightPosition - _position);
	float Id = max(dot(lightDirection, _normal), 0.0);
	
	vec3 viewDirection = normalize(_viewPosition - _position);
	vec3 reflectDirection = reflect(-lightDirection, _normal);
	float Is = pow(max(dot(viewDirection, reflectDirection), 0.0), shininess);
	
	fragColor = vec4((Ia + Id) * diffuseColor + Is * specularColor, 1.0);
}
)""
