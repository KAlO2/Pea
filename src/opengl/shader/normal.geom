R""(
layout(location = 1) uniform mat4 viewProjection;
layout(location =11) uniform float length;

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in vec3 _normal[];

void main()
{
	vec4 position = gl_in[0].gl_Position;
	
	gl_Position = viewProjection * position;
	EmitVertex();
	
	gl_Position = viewProjection * (position + vec4(length * _normal[0], 0.0));
	EmitVertex();
	
	EndPrimitive();
}
)""
