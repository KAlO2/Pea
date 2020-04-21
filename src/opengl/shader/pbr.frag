R""(
const int N = 4;
layout(location = 16, binding = 0) uniform sampler2D texture0;
uniform vec3 lightPositions[N];
uniform vec3 lightColors[N];
uniform vec3 viewPosition;

in vec3 _position;
in vec2 _texcoord;
in vec3 _normal;

out vec4 fragColor;

const float PI = 3.14159265358979323846264338327950288;

/**
 * http://filmicworlds.com/blog/optimizing-ggx-shaders-with-dotlh/
 * geometry function tells us the percentage of surface points with N = H that are not shadowed or 
 * masked, as a function of the light direction L and the view direction V.
 * @param[in] F0 represents the base reflectivity of the surface, which we calculate using something
 *               called the indices of refraction or IOR.
 */
float lightingFunctionGGX(vec3 N, vec3 V, vec3 L, float roughness, float F0)
{
	vec3 H = normalize(V + L);
	
	// The normal distribution function D statistically approximates the relative surface area of 
	// microfacets exactly aligned to the (halfway) vector H.
	// Trowbridge-Reitz GGX
	float a = roughness * roughness;
	float aa = a * a;
	float NdotH = dot(N, H);
	float LdotH = dot(L, H);
	NdotH = max(NdotH, 0.0);
	float r = NdotH * NdotH *(aa - 1.0) + 1.0);
	// divide by zero case: if roughness=0.0 and NdotH=1.0, r = 0.0;
	float D = aa / (PI * r * r);
	
	// Fresnel Schlick
	// F(F0, L, H) = F0 + (1 - F0)(1 - dot(L, H))^5
	float F = F0 + (1 - F0) * pow(1 - LdotH, 5.0);
	
	// The geometry function statistically approximates the relative surface area where its micro 
	// surface-details overshadow each other causing light rays to be occluded.
	float r = roughness + 1.0;
	float k = r * r / 8.0;
	float NdotV = dot(N, V);
	float G = NdotV / (NdotV * (1.0 - k) + k);
}

vec3 irradiance(vec3 lightPosition, vec3 lightColor)
{
	vec3 direction = lightPosition - _position;
//	float distance = length(direction);
//	float attenuation = 1.0 / (distance * distance);
	float attenuation = 1.0 / dot(direction, direction);
	vec3f radiance = lightColor * attenuation;
	
	// Cook-Torrance BRDF
	float D = DistributionGGX(_normal, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	
	vec3 nominator = D * G * F;
	float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
	
}

void main()
{
	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);
	
	for(int i = 0; i < N; ++i)
		irradiance(lightPositions[i], lightColors[i]);
	vec3 color = texture(texture0, _texcoord).rgb;
	fragColor = vec4(color, 1.0);
}
)""
