R"(
#if USE_DYNAMIC_LIGHTS
layout(location = 5) uniform samplerBuffer bufferLightPositions;
layout(location = 7) uniform samplerBuffer bufferLightColors;
#else
// LIGHT_COUNT must be defined here
uniform vec3 lightPositions[LIGHT_COUNT];
uniform vec3 lightColors[LIGHT_COUNT];
#endif

layout(location = 8) uniform vec3 viewPosition;

#if USE_TEXTURE
uniform sampler2D textureAlbedo;
uniform sampler2D textureNormal;
uniform sampler2D textureMetallic;
uniform sampler2D textureRoughness;
uniform sampler2D textureAo;

in vec2 _texcoord;
#else
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;
#endif

in vec3 _position;
in vec3 _normal;

out vec4 fragmentColor;

const float PI = 3.14159265358979323846264338327950288;
const float EPS = 1E-7;

// formulas are from s2013_pbs_epic_notes_v2.pdf
/*
 * For the normal distribution function (NDF), we found Disney’s choice of GGX/Trowbridge-Reitz to
 * be well worth the cost.
 *                          alpha^2
 * D(h) = --------------------------------------
 *         pi * ((N * H)^2 (alpha^2 - 1) + 1)^2
 */
float distributionGGX(float NdotH, float roughness)
{
	float alpha = roughness * roughness;
	float aa = alpha * alpha;
	float r = NdotH * NdotH * (aa - 1.0) + 1.0;
	float den = PI * (r * r);
	return aa / max(den, EPS);  // to prevent divided by zero for roughness=0.0 and NdotH=1.0
}

/*
 *      (roughness + 1)^2                  N * V
 * k = ------------------ , G(v) = ------------------, G(L, V, H) = G(L) * G(v)
 *             8                   (N * V)(1 - k) + k
 */
float geometrySchlickGGX(float VdotN, float roughness)
{
	float r = (roughness + 1.0);
	float k = r * r / 8.0;
	return VdotN / mix(VdotN, 1.0, k);
}

float geometrySmith(float LdotN, float VdotN, float roughness)
{
	float ggx1 = geometrySchlickGGX(LdotN, roughness);
	float ggx2 = geometrySchlickGGX(VdotN, roughness);
	return ggx1 * ggx2;
}

/*
 * F = F0 + (1 - F0) * (1 - cosTheta)^5
 */
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	float x = 1 - cosTheta;
	float xx = x * x;
	float x5 = xx * xx * x;
	return mix(F0, vec3(1.0), x5);
}

void main()
{
#if USE_TEXTURE
	vec3  albedo    = texture(textureAlbedo,    _texcoord).rgb;
	float metallic  = texture(textureMetallic,  _texcoord).r;
	float roughness = texture(textureRoughness, _texcoord).r;
	float ao        = texture(textureAo,        _texcoord).r;
	
	// convert normal from tangent space to world space
	vec3 normal = texture(textureNormal, _texcoord).xyz * 2.0 - 1.0;  // MAD [0,1] => [-1,1]
	vec3 dp1 = dFdx(_position), dp2 = dFdy(_position);
	vec2 dt1 = dFdx(_texcoord), dt2 = dFdy(_texcoord);
	// dp1 = dt1.s * T + dt1.v * B
	// dp2 = dt2.s * T + dt2.v * B
	// solve the equation, we get
	//     | dt1.u   dt1.v |                 1    |  dt2.v   -dt1.v |
	// A = |               |  ==>  A^-1 = ------- |                 |
	//     | dt2.u   dt2.v |               det(A) | -dt2.u    dt1.u |
	//
	// in which det(A) = dt1.u * dt2.v - dt1.v * dt2.u
	float det = dt1.s * dt2.t - dt1.t * dt2.s;
	vec3 T  = ( dt2.t * dp1 - dt1.t * dp2) / det;
//	vec3 B  = (-dt2.s * dp1 + dt1.s * dp2) / det;
	vec3 B  = cross(_normal, T);
	mat3 TBN = mat3(T, B, _normal);
	
	vec3 N = TBN * normal;
#else
	vec3 N = _normal;
#endif
	
	vec3 V = normalize(viewPosition - _position);
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 color = vec3(0.0);
#if USE_DYNAMIC_LIGHTS
	const int lightCount = textureSize(bufferLightPositions);
	for(int i = 0; i < lightCount; ++i) 
	{
		vec3 lightPosition = texelFetch(bufferLightPositions, i).xyz;
		vec3 lightColor = texelFetch(bufferLightColors, i).rgb;
#else
	for(int i = 0; i < LIGHT_COUNT; ++i)
	{
		vec3 lightPosition = lightPositions[i];
		vec3 lightColor = lightColors[i];
#endif
		vec3 L = normalize(lightPosition - _position);
		vec3 H = normalize(V + L);
		
		// attenuation = 1 / (constant * x + linear * x + quadratic * x^2)
		vec3 attenuationFactor = vec3(1.0, 0.0, 0.25);  // can be tweaked
		vec3 direction = lightPosition - _position;
		float distanceSquared = dot(direction, direction);
		float distance = sqrt(distanceSquared);
		float attenuation = 1.0 / dot(vec3(1.0, distance, distanceSquared), attenuationFactor);
		vec3 radiance = lightColor * attenuation;

		float VdotN = max(dot(V, N), 0.0);
		float LdotN = max(dot(L, N), 0.0);
		float HdotN = max(dot(H, N), 0.0);
		float VdotH = max(dot(V, H), 0.0);  // or dot(L, H)
		
		// Cook-Torrance BRDF
		float D = distributionGGX(HdotN, roughness);
		vec3  F = fresnelSchlick(F0, VdotH);
		float G = geometrySmith(LdotN, VdotN, roughness);
		
		vec3  num = D * F * G; 
		float den = 4 * VdotN * LdotN;
		vec3 specular = num / max(den, EPS);  // to prevent divided by zero for VdotN=0.0 or LdotN=0.0
		
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;  // pure metals have no diffuse light
		color += (kD * albedo / PI + specular) * radiance;
	}
	
	// TODO: replace ambient lighting with environment lighting
	vec3 ambient = vec3(0.03) * albedo * ao;
	color += ambient;
	
	color = color / (color + vec3(1.0));  // HDR tone mapping
	color = pow(color, vec3(1.0 / 2.2));  // gamma correction

	fragmentColor = vec4(color, 1.0);
}
)";

