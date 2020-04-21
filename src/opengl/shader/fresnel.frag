#version 150

in vec3 reflect_;
in float ratio;
in vec3 refract_r;
in vec3 refract_g;
in vec3 refract_b;

uniform samplerCube cube_map;

out vec4 out_color;

void main()
{
	vec3 refract_color;
	refract_color.r = vec3(texture(cube_map, refract_r)).r;
	refract_color.g = vec3(texture(cube_map, refract_g)).g;
	refract_color.b = vec3(texture(cube_map, refract_b)).b;
	
	vec3 reflect_color = vec3(texture(cube_map, reflect_));
	
	out_color = vec4(mix(refract_color, reflect_color, ratio), 1.0);
}