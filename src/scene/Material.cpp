#include "scene/Material.h"

#include "opengl/Program.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"

using namespace pea;

Material::Material():
		Material("")
{

}

Material::Material(const std::string& name):
		name(name),
		ambient(0.0f),
		diffuse(0.0f),
		specular(0.0f),
		emissive(0.0f),
		shininess(1.0F),
		transmittance(0.0f),
		ior(1.0F),
		dissolve(1.0F),
		illum(IlluminationMode::HIGHLIGHT_ON)
{
	
}

Material::~Material()
{
//	std::cout << "material \"" << name << "\" destructed" << '\n';
}
/*
std::vector<std::shared_ptr<Texture>> Material::loadTexture(const std::string& dir) const
{
}
*/


void Material::clear()
{
	name = "";
	constexpr vec3f ZERO(0.0F);
	ambient = ZERO;
	diffuse = ZERO;
	specular = ZERO;
	shininess = 1.0F;
	
	illum = IlluminationMode::HIGHLIGHT_ON;

	ambient_texname.clear();
	diffuse_texname.clear();
	specular_texname.clear();
//	normal_texname.clear();
}

