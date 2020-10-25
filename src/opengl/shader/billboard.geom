R""(
//layout(location = 0) uniform vec3 objectPosition;
layout(location = 1) uniform mat4 viewProjection;
layout(location = 8) uniform vec3 viewPosition;

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in flat vec3 _color[];
in vec2 _size[];

out vec2 texcoord;
out flat vec3 color;

void main()
{
	vec3 position = /*objectPosition +*/ gl_in[0].gl_Position.xyz;
	vec3 forward = normalize(position - viewPosition);
	vec3 worldUp = vec3(0.0, 0.0, 1.0);
	vec3 right = cross(forward, worldUp);
	vec3 up = cross(right, forward);
	
	// 0--2    0: -+
	// | /|    1: --
	// |/ |    2: ++
	// 1--3    3: +-
	vec2 halfSize = _size[0] * 0.5;
	vec3 h = halfSize.x * right;
	vec3 v = halfSize.y * up;
	
	vec3 position0 = position - h + v;
	gl_Position = viewProjection * vec4(position0, 1.0);
	texcoord = vec2(0.0, 1.0);
	color = _color[0];
	EmitVertex();
	
	vec3 position1 = position - h - v;
	gl_Position = viewProjection * vec4(position1, 1.0);
	texcoord = vec2(0.0, 0.0);
	color = _color[0];
	EmitVertex();
	
	vec3 position2 = position + h + v;
	gl_Position = viewProjection * vec4(position2, 1.0);
	texcoord = vec2(1.0, 1.0);
	color = _color[0];
	EmitVertex();
	
	vec3 position3 = position + h - v;
	gl_Position = viewProjection * vec4(position3, 1.0);
	texcoord = vec2(1.0, 0.0);
	color = _color[0];
	EmitVertex();
	
	EndPrimitive();
}
)""
