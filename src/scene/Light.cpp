#include "scene/Light.h"

using namespace pea;

const char* PointLight::SOURCE = R""(
struct PointLight
{
	vec3f ambient;
	vec3f diffuse;
	vec3f specular;
	
	vec3 position;
	vec3 attenuation;
	float range;
};
)"";

const char* DirectionalLight::SOURCE = R""(
struct DirectionalLight
{
	vec3f ambient;
	vec3f diffuse;
	vec3f specular;
	vec3f direction;
};
)"";

const char* SpotLight::SOURCE = R""(
struct SpotLight
{
	vec3f ambient;
	vec3f diffuse;
	vec3f specular;
	
	vec3 position;
	vec3 attenuation;
	vec3 direction;
	vec3 falloff;
};
)"";

Light::Light(Light::Type type):
		type(type),
		ambient(0.0F, 0.0F, 0.0F),
		diffuse(1.0F, 1.0F, 1.0F),
		specular(1.0F, 1.0F, 1.0F)
{

}

PointLight::PointLight(PointLight::Type type, const vec3f& position):
		Light(type),
		position(position),
		attenuation(1.0, 0.0, 0.0),
		range(1E3)  // std::numeric_limit<float>::max()
{
}

PointLight::PointLight(const vec3f& position):
		PointLight(Light::Type::POINT, position)
{
}
/*
void Light::setUniform(uint32_t program, const std::string& uniform) const
{
	assert(program != 0);

	std::string uniform_ = std::string(uniform) + '.';
	auto setMember = [&program, &uniform_](const char* name, const vec3f& value)
	{
		std::string uniform_name = uniform_ + name;
		int32_t location = Program::getUniformLocation(program, uniform_name);
		assert(location != Shader::INVALID_LOCATION);
		if(location != Shader::INVALID_LOCATION)
			Program::setUniform(location, value);
	};
	
	setMember("ambient", ambient);
	setMember("diffuse", diffuse);
	setMember("specular", specular);
	
}

void PointLight::setUniform(uint32_t program, const std::string& uniform) const
{
	Light::setUniform(program, uniform);
	
	std::string uniform_position = uniform + '.' + "position";
	int32_t location = Program::getUniformLocation(program, uniform_position);
	assert(location != Shader::INVALID_LOCATION);
	Program::setUniform(location, position);
}
*/
DirectionalLight::DirectionalLight(const vec3f& direction):
		Light(Light::Type::DIRECTIONAL),
		direction(direction)
{
}

SpotLight::SpotLight(const vec3f& position, const vec3f& direction):
		PointLight(Light::Type::SPOT, position),
		direction(direction),
		innerAngle(0.0F),
		outerAngle(M_PI / 4),
		falloff(1.0F)
{
}


