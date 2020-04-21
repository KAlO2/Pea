#ifndef PEA_IO_MODEL_MTL_H_
#define PEA_IO_MODEL_MTL_H_

#include <memory>

#include "scene/Material.h"

namespace pea {

/**
 * .MTL (Material Template Library) file specification
 * see {@link http://www.fileformat.info/format/material/ }
 * see {@link http://paulbourke.net/dataformats/mtl/ }
 */
class Model_MTL final
{
public:
/*
	std::unordered_map<std::string, size_t> table;
	std::vector<Material> materials;

	store Material in array, with -1 being invalid material.
	In contract, We can use map, with empty string being invalid material.
*/
/*
	enum Key
	{
//		Material name statement:
		KEY_MATERIAL = 0,         // "newmtl"

//		Material color and illumination statements:
		KEY_AMBIENT,              // "Ka"
		KEY_DIFFUSE,              // "Kd"
		KEY_SPECULAR,             // "Ks"
		KEY_TRANSMISSION_FILTER,  // "Tf"
		KEY_ILLUMINATION,         // "illum"
		KEY_DISSOLVE,             // "d"
		KEY_DISSOLVE_HALO,        // "d -halo"
		KEY_SPECULAR_EXPONENT,    // "Ns"
		KEY_SHARPNESS,            // "sharpness"
		KEY_OPTICAL_DENSITY,      // "Ni"

//		Texture map statements:
		KEY_AMBIENT_TEXTURE,      // "map_Ka"
		KEY_DIFFUSE_TEXTURE,      // "map_Kd"
		KEY_SPECULAR_TEXTURE,     // "map_Ks"
//		KEY_MAP_NS,               // "map_Ns"
//		KEY_MAP_D,                // "map_d"
//		KEY_DECAL,                // "decal"
//		KEY_BUMP_TEXTURE,         // "disp"
//		KEY_BUMP,

//		Reflection map statement:
//		KEY_REFLECTION,           // "relf"


//		KEY_REFL,
		KEY_COUNT,
	};
*/
private:
//	static const Property property[KEY_COUNT];

	const std::string path;
	std::unordered_map<std::string, std::shared_ptr<Material>> materials;

public:
	static std::string toString(const Material* material);
	
public:
	Model_MTL(const std::string& path) noexcept(false);
	~Model_MTL() = default;

	Model_MTL(const Model_MTL& other) = delete;
	Model_MTL& operator =(const Model_MTL& other) = delete;
	
	Model_MTL(Model_MTL&& other);
	Model_MTL& operator = (Model_MTL&& other) = delete;
	
	std::unordered_map<std::string, std::shared_ptr<Texture>> loadTexture() const;
	
	size_t getSize() const;
	
	bool contains(const std::string& name);
	
	std::shared_ptr<Material> at(const std::string& name) const;
	
	bool save(const std::string& path) const;
	
	std::string toString() const;
	
	void clear();
};

}  // namespace pea
#endif  // PEA_IO_MODEL_MTL_H_
