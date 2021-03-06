R""(
//layout(std140) uniform PointLight
struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

layout(location = 3) uniform vec4 uniformColor;
uniform PointLight light0;
layout(location = 4) uniform vec3 viewPosition;
//layout(location = 5) uniform vec3 lightPosition;
//layout(location = 10) uniform vec3 lightColor;

in vec3 _position;
flat in vec3 _normal;

out vec4 fragColor;

void main()
{
	vec3 incident = normalize(_position - light0.position);
	// dot(normal, -incident) == -dot(_normal, incident)
	float Ld = max(-dot(_normal, incident), 0.0);
	
	vec3 diffuse = Ld * uniformColor.rgb * light0.diffuse;
	
	fragColor = vec4(diffuse, uniformColor.a);
}
)""
