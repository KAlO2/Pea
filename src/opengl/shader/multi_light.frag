#version 330

enum Type
{
	TYPE_POINT,
	TYPE_SPOT,
	TYPE_DIRECTIONAL,
	
	TYPE_COUNT
};

// ONLY one directional light!
const int MAX_LIGHT[TYPE_COUNT] = { 2, 2, 1 };

struct BaseLight
{
	vec3 color;
	float ambient_intensity; 
	float diffuse_intensity;
}

struct PointLight
{
	BaseLight base;
	vec3 position;
	vec3 attenuation;
}

struct SpotLight
{
	PointLight point;
	vec3 direction;
float cutoff;
};

struct DirectionalLight
{
BaseLight Base;
vec3 direction;
};

struct PointLight
{
	vec3 color;
	vec3 position;
	
	float ambient;
	vec3 attenuation;
};

vec4 calculateLight(in BaseLight light, in vec3 direction, in vec3 normal)
{
	vec4 ambient = vec4(light.color * light.ambient_intensity, 1.0f);
	float diffuse_factor = dot(normal, -direction);
	
	vec4 sum = ambient;
	if(diffuse_factor > 0)  // in sight
	{
		vec4 diffuse_color = vec4(light.color * light.diffuse_intensity * diffuse_factor, 1.0f);
		sum += diffuse_color;
		
		vec3 vertex;
		float specular_factor = dot();
		if(specular_factor > 0)
		{
			
		}
	}
	
	return sum;
}

struct SpotLight
{
	vec3 color;
	vec3 position;
	vec3 direction;
	
	bool enabled;  // whether spotlight is on
	
	// If cone angle is set from program, we also provide its cosine value,
	// so that it isn't always recalculated.
	float angle, cos_angle;
	
	// Only linear attenuation, you can code const and exp as well
	float fLinearAtt;
};


uniform int num_light[TYPE_COUNT];

vec4 getPointLightColor(in const PointLight light, in vec3 world_position, in vec3 normal)
{
	vec3 direction = light.position - world_position;
	float distance = length(direction);
	direction = normalize(direction);
	
	float diffuse = max(0.0, dot(normal, direction));
	const vec3f& attenuation = light.attenuation;  // (x, y, z) (constant, linear, quadratic)
	float decay = attenuation.x + attenuation.y*distance + attenuation.z*distance*distance;

	float factor = (light.ambient + diffuse)/decay;
	return vec4(light.color * factor, 1.0);
}

vec4 getSpotLightColor(const SpotLight light, in vec3 world_position)
{
	const vec4 black(0.0, 0.0, 0.0, 1.0);
	// If flashlight isn't turned on, return black color
	if(spotLight.enabled)
		return black;

	vec3 direction = world_position - light.position;
	float distance = length(direction);
	direction = normalize(direction);

	// Cosine between spotlight direction and directional vector to fragment
	float cosine = dot(light.direction, direction);
	if(cosine < light.cos_angle)  // we're outside the cone
		return dark;

	// How strong the light is depends whether its nearer to the center of 
	// cone or away from cone borders.
	float factor = (cosine - light.cos_angle)/(1.0 - light.cos_angle));

	float decay = attenuation.x + attenuation.y*distance + attenuation.z*distance*distance;
return vec4(light.color * factor/decay, 1.0);
}

vec4 calculate()
void main()
{
	vec3 normal = normalize(normal0);
	vec4 TotalLight = CalcDirectionalLight(Normal);
	
	for (int i = 0; i < num_light[TYPE_POINT]; ++i)
	TotalLight += CalcPointLight(gPointLights[i], Normal);
	}
	
	for (int i = 0 ; i < gNumSpotLights ; i++) {
	TotalLight += CalcSpotLight(gSpotLights[i], Normal);
	}   
	
	FragColor = texture(gColorMap, TexCoord0.xy) * TotalLight;
}
