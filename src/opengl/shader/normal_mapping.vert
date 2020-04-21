R""(
layout(location = 0) uniform mat4 model;
layout(location = 4) uniform mat4 viewProjection;
layout(location =14) uniform vec3 viewPosition;
layout(location =15) uniform vec3 lightPosition;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 5) in vec3 tangent;
layout(location = 6) in vec3 bitangent;
layout(location = 8) in vec2 texcoord;

out vec3 _lightPosition;
out vec3 _viewPosition;
out vec3 _position;
out vec3 _normal;
out vec2 _texcoord;

void main()
{
	// transform position to worldspace
	vec4 modelPosition = model * vec4(position, 1.0);
	_position = vec3(modelPosition);
	
	// transform normal
	mat3 normalTransform = transpose(inverse(mat3(model)));
	vec3 T = normalTransform * tangent;
	vec3 B = normalTransform * bitangent;
	vec3 N = normalTransform * normal;
	
	// for three normalized perpendicular vector,
	// transpose == inverse, but transpose() is much cheaper to calcualate.
	mat3 TBN = transpose(mat3(T, B, N));
	_position      = TBN * _position;
	_viewPosition  = TBN * viewPosition;
	_lightPosition = TBN * lightPosition;
	
	_texcoord = texcoord;
	
	gl_Position = viewProjection * modelPosition;
}
)""
