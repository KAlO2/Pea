#version 150

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 camera_position;

const vec3 eta = vec3(0.64, 0.65, 0.66);
const float fresnel_power = 6.0;

const float f = ((1.0 - eta.g)*(1.0 - eta.g)) / ((1.0 + eta.g)*(1.0 + eta.g));

out vec3 reflect_;
out float ratio;
out vec3 refract_r;
out vec3 refract_g;
out vec3 refract_b;


void main()
{
	vec3 incident = normalize(position - camera_position);
	
	ratio = f + (1.0 - f) * pow((1.0 - dot(-incident, normal)), fresnelPower);
	
	refract_r = refract(incident, normal, eta.r);
	refract_g = refract(incident, normal, eta.g);
	refract_b = refract(incident, normal, eta.b);
	
	reflect_ = reflect(incident, normal);
	
	gl_Position = projection * view * model * vec4(position, 1);
}