R"(
#if USE_MULTIPLE_LIGHTS
layout(location = 5) uniform samplerBuffer bufferLightPositions;
layout(location = 7) uniform samplerBuffer bufferLightColors;
#else
layout(location = 5) uniform vec3 lightPosition;
layout(location = 7) uniform vec3 lightColor;
#endif

layout(location = 8) uniform vec3 viewPosition;

#if USE_TEXTURE
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

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
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	float x = 1 - cosTheta;
	float xx = x * x;
	float x5 = xx * xx * x;
	return mix(F0, vec3(1.0), x5);
}

void main()
{
#if USE_TEXTURE
	vec3  albedo    = texture(albedoMap,    _texcoord).rgb;
	float metallic  = texture(metallicMap,  _texcoord).r;
	float roughness = texture(roughnessMap, _texcoord).r;
	float ao        = texture(aoMap,        _texcoord).r;
	
	// convert normal from tangent space to world space
	vec3 normal = texture(normalMap, _texcoord).xyz * 2.0 - 1.0;  // MAD [0,1] => [-1,1]
	vec3 dp1 = dFdx(_position), dp2 = dFdy(_position);
	vec2 dt1 = dFdx(_texcoord), dt2 = dFdy(_texcoord);
	// dp1 = dt1.s * T + dt1.v * B
	// dp2 = dt2.s * T + dt2.v * B
	// solve the equation, we get
	//     | dt1.s  dt1.v |                 1    |  dt2.v  -dt1.v |
	// A = |              |  ==>  A^-1 = ------- |                |
	//     | dt2.s  dt2.v |               det(A) | -dt2.s   dt1.s |
	//
	// in which det(A) = dt1.s * dt2.v - dt1.v * dt2.s
	float det = dt1.s * dt2.v - dt1.v * dt2.s;
	vec3 T  = ( dt2.v * dp1 - dt1.v * dp2) / det;
//	vec3 B  = (-dt2.s * dp1 + dt1.s * dp2) / det;
	vec3 B  = cross(_normal, T);
	mat3 TBN = mat3(T, B, _normal);
	vec3 N = TBN * normal;
#endif

	vec3 N = _normal;
	vec3 V = normalize(viewPosition - _position);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	vec3 F0 = vec3(0.04); 
	F0 = mix(F0, albedo, metallic);

#if USE_MULTIPLE_LIGHTS
	vec3 color = vec3(0.0);
	const int lightCount = 3;//textureSize(bufferLightPositions);
	for(int i = 0; i < lightCount; ++i) 
	{
		vec3 lightPosition = texelFetch(bufferLightPositions, i).xyz;
		vec3 lightColor = texelFetch(bufferLightColors, i).rgb;
#endif
		vec3 L = normalize(lightPosition - _position);
		vec3 H = normalize(V + L);
//		float distance = length(lightPositions[i] - _position);
//		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColor;// * attenuation;

		float VdotN = max(dot(V, N), 0.0);
		float LdotN = max(dot(L, N), 0.0);
		float HdotN = max(dot(H, N), 0.0);
		float VdotH = max(dot(V, H), 0.0);  // or dot(L, H)
		
		// Cook-Torrance BRDF
		float D = distributionGGX(HdotN, roughness);
		vec3  F = fresnelSchlick(VdotH, F0);
		float G = geometrySmith(LdotN, VdotN, roughness);
		
		vec3  num = D * F * G; 
		float den = 4 * VdotN * LdotN;
		vec3 specular = num / max(den, EPS);  // to prevent divided by zero for NdotV=0.0 or NdotL=0.0

		// kS is equal to Fresnel
		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;  // pure metals have no diffuse light

		vec3 Lo = (kD * albedo / PI + specular) * radiance;
#if USE_MULTIPLE_LIGHTS
		color += Lo;
	}
#else
		color = Lo;
#endif
	
	fragmentColor = vec4(color, 1.0);
}
)";

