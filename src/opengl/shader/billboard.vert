R""(
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 7) in vec2 size;

out flat vec3 _color;
out vec2 _size;

void main()
{
	gl_Position = vec4(position, 1.0);
	_color = color;
	_size  = size;
}
)""
