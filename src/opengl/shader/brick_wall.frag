R""(

in vec2 _texcoord;

out vec4 fragColor;

const float BRICK_SIZE = vec2(0.25, 0.08);
const float MORTAR_THICKNESS = 0.01;

vec3 BRICK_COLOR = vec3(0.50, 0.15, 0.14);
vec3 MORTAR_COLOR = vec3(0.50, 0.50, 0.50);

void main()
{
	vec2 block = BRICK_SIZE + MORTAR_THICKNESS;
	_texcoord /= block;
	if(_texcoord.t % (2 * block.y) > block.y)
		_texcoord.s = (_texcoord.s + 0.5 * block.x) % block.x;
	
	vec2 lb(MORTAR_THICKNESS, MORTAR_THICKNESS);
	vec2 rt = lb + BRICK_SIZE;
	bool isBrick = lb < _texcoord && _texcoord < rt;
	vec3 color = isBrick ? BRICK_COLOR: MORTAR_COLOR
	fragColor = vec4(color, 1.0);
}
)""
