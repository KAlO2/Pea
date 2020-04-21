R""(
layout(location = 17) uniform float length;

layout(points) in;
layout(line_strip, max_vertices = 2) out;

in vec3 _normal[];

void main()
{
	vec4 position = gl_in[0].gl_Position;
	
	gl_Position = position;
	EmitVertex();
	
	gl_Position = position + vec4(length * _normal[0], 0.0);
	EmitVertex();
	
	EndPrimitive();
}
)""
