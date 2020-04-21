R""(
in vec3 vertex_position;

in  vec4 position_texcoord;
out vec2 texcoord;

/*
	model_view_projection, abbreviated to MVP, 
	for col_major: MVP = P*V*M; gl_Position = MVP * vertex_position;
	for row_major: MVP = M*V*P; gl_Position = vertex_position * MVP;
*/
uniform mat4 model_view_projection;

void main()
{
	// output position of the vertex, in clip space
	gl_Position =  model_view_projection * vec4(vertex_position, 1.0);
	
	texcoord = position_texcoord.zw;
}
)""
