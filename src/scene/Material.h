#ifndef PEA_SCENE_MATERIAL_H_
#define PEA_SCENE_MATERIAL_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "math/vec3.h"

namespace pea {

class Texture;


enum class IlluminationMode: uint32_t
{
	// http://people.cs.clemson.edu/~dhouse/courses/405/docs/brief-mtl-file-format.html
	/**
	 * This is a constant color illumination model. The color is the specified
	 * Kd for the material. The formula is: color = Kd
	 */
	COLOR_ON_AND_AMBIENT_OFF = 0,

	/**
	 * illum = 1 a flat material with no specular highlights
	 * This is a diffuse illumination model using Lambertian shading. The color
	 * includes an ambient and diffuse shading terms for each light source. The
	 * formula is:  color = Ka*Ia + Kd*{ SUM j=1..ls, (N * Lj)Ij }
	 */
	COLOR_ON_AND_AMBIENT_ON = 1,

	/**
	 * illum = 2 denotes the presence of specular highlights
	 * This is a diffuse and specular illumination model using Lambertian shading and Blinn's interpretation of Phong's specular illumination model (BLIN77).
The color includes an ambient constant term, and a diffuse and specular shading term for each light source. The formula is:
  color = KaIa + Kd { SUM j=1..ls, (N*Lj)Ij } + Ks { SUM j=1..ls, ((H*Hj)^Ns)Ij }

  Term definitions are: Ia ambient light, Ij light j's intensity, Ka ambient reflectance, Kd diffuse reflectance, Ks specular reflectance, H unit vector bisector between L and V, L unit light vector, N unit surface normal, V unit view vector
	 */
	HIGHLIGHT_ON = 2,
	
	REFLECTION_ON_AND_RAY_TRACE_ON = 3,
	TRANSPARENCY_GLASS_ON_REFLECTION_RAY_TRACE_ON = 4,
	REFLECTION_FRESNEL_ON_AND_RAY_TRACE_ON = 5,
	TRANSPARENCY_REFRACTION_ON_REFLECTION_FRESNEL_OFF_AND_RAY_TRACE_ON = 6,
	TRANSPARENCY_REFRACTION_ON_REFLECTION_FRESNEL_ON_AND_RAY_TRACE_ON = 7,
	REFLECTION_ON_AND_RAY_TRACE_OFF = 8,
	TRANSPARENCY_GLASS_ON_REFLECTION_RAY_TRACE_OFF = 9,
	CASTS_SHADOWS_ONTO_INVISIBLE_SURFACES = 10,
	
	
};


class Material
{
	friend class Model_MTL;
public:
	enum Type : int32_t
	{
		TYPE_NONE = 0,

		TYPE_AMBIENT,
		TYPE_DIFFUSE,
		TYPE_SPECULAR,
		TYPE_EMISSIVE,
		TYPE_SHININESS,
//		TYPE_AMBIENT_AND_DIFFUSE,

		TYPE_HEIGHT,  // bump mapping
		TYPE_NORMAL,
		TYPE_DISPLACEMENT,


		TYPE_OPACITY,
	};

	/* @see http://en.wikipedia.org/wiki/List_of_common_shading_algorithms */
	enum ShadingMode
	{
		SHADING_FLAT,
		SHADING_GOURAUD,

		/* Ambient + Diffuse + Specular = Phone Reflection
		 *
		 * Material: Ambient (Ka); Diffuse (Kd); Specular (Ks); Shininess (Ns).
		 * Light: Ambient (Ia); Diffuse (Id); Specular (Is).
		 * Vectors: Light (L); Normal (N); Reflection (R); Viewer (V).
		 *
		 * L, which is the directional vector from the surface point to the light source.
		 * N, which is the normal of the surface point.
		 * R, which is the reflection of the ray of light L against the normal N.
		 * V, which is the direction from the surface point towards the viewer.
		 *
		 *  For a surface point Ip, is:
		 *  Ip = Ka*Ia + Kd*Id*(L*N) + Ks*Is*(R*V)^Ns
		 */
		SHADING_PHONG,
	};

private:
	enum
	{
		AMBIENT  = 0,
		DIFFUSE  = 1,
		SPECULAR = 2,
	};
public:

	std::string name;

	vec3f ambient;
	vec3f diffuse;
	vec3f specular;
	vec3f emissive;
	float shininess;
	vec3f transmittance;

	float ior;      // index of refraction
	float dissolve; // 1 == opaque; 0 == fully transparent

	IlluminationMode illum; // illumination model

	// texture filenames
	std::string ambient_texname;            // map_Ka
	std::string diffuse_texname;            // map_Kd
	std::string specular_texname;           // map_Ks
	std::string specular_highlight_texname; // map_Ns
	std::string bump_texname;               // map_Bump, bump
	std::string displacement_texname;       // disp
	std::string alpha_texname;              // map_d

	std::unordered_map<std::string, std::string> unknownParameters;

public:
	Material();
	Material(const std::string& name);
	virtual ~Material();

//	std::vector<std::shared_ptr<Texture>> loadTexture(const std::string& dir) const;
	
//	void setProgram(uint32_t program, const std::string& uniform) const;
	
	void clear();
	
};

}  // namespace pea
#endif  // PEA_SCENE_MATERIAL_H_
