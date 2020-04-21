#ifndef PEA_OPENGL_SHADER_FACTORY_H_
#define PEA_OPENGL_SHADER_FACTORY_H_

#include <string>
#include <cstdint>
#include <unordered_map>

#include "opengl/Shader.h"

namespace pea {

/**
 * Creates Shader objects from various sources, including files, streams,
 * and byte-arrays.
 */
class ShaderFactory final
{
public:
	static const std::string VERSION;
	
	// Built-in Shaders
	enum Index: uint32_t
	{
		VERTEX_SHADER_START = 0,
		VERT_POSITION = 0,             // position
		VERT_MVP,                      // position, modelViewProjection
		VERT_M_VP,                     // position, model, viewProjection
		VERT_M_V_P,                    // position, model, view, projection
		VERT_V_P,                      // position, view, projection
		VERT_VP,                       // position, viewProjection
		
		VERT_TEXCOORD,                 // vec2 position, texcoord
		VERT_M_V_P_COLOR,              // position, model, view, projection, color
		VERT_M_V_P_TEXCOORD,           // position, model, view, projection, texcoord
		VERT_M_VP_TEXCOORD,            // position, model, viewProjection, texcoord
		VERT_M_VP_TEXCOORD_INSTANCE0,  // position, model, viewProjection, texcoord, vec4 instance
		VERT_M_VP_TEXCOORD_INSTANCE1,  // position, model, viewProjection, texcoord, mat4 instance
		VERT_M_VP_COLOR,               // position, model, viewProjection, color_rgb
		VERT_M_VP_RGBA,                // position, model, viewProjection, color_rgba
		VERT_BILLBOARD,                // position, size, color_rgb, flat shadding
//		VERT_M_V_P_TEXCOORD_COLOR,     // position, model, view, projection, texcoord, color
		
		VERT_M_V_P_NORMAL,             // position, model, view, projection, normal
		VERT_M_VP_NORMAL,              // position, model, viewProjection, normal
		VERT_M_VP_NORMAL_FLAT,         // ditto, with flat shadding
		VERT_M_VP_TEXCOORD_NORMAL,     // position, model, viewProjection, texcoord, normal
		VERT_M_VP_SPHERE_NORMAL,       // position, model, viewProjection => sphere normal
		VERT_M_VP_SPHERE_TEXCOORD,     // position, model, viewprojection => sphere texcoord

		VERT_NORMAL_MAPPING,
		VERT_TEXTURE_FONT,
		VERT_SKYBOX,
		VERT_MOTION_BLUR,
		
		
		TESS_CONTROL_SHADER_START = 100,
		
		TESS_EVALUATION_SHADER_START = 200,
		
		GEOMETRY_SHADER_START = 300,
		GEOM_BILLBOARD = 300,
		GEOM_NORMAL,  // draw vertex normal or face normal
		
		FRAGMENT_SHADER_START = 400,
		FRAG_UNIFORM_COLOR = 400,  // uniform (omnidirectional) scattering
		FRAG_LAMBERT,
		FRAG_LAMBERT_FLAT,  // ditto, with flat shadding
		FRAG_TEXTURE_LUMINANCE,
//		FRAG_TEXTURE_LUMINANCE_ALPHA,
		FRAG_TEXTURE_RGB,
		FRAG_TEXTURE_RGBA,
		FRAG_TEXTURE_COLOR_ALPHA,  // texture_rgb, float alpha
		FRAG_TEXTURE_ALPHA_COLOR,  // texture_a, vec3 color;
		FRAG_TEXTURE_RGBA_ALPHA,
		FRAG_TEXTURE_DEPTH,
		FRAG_TEXTURE_FONT,
		FRAG_TEXTURE_CUBE,
		FRAG_COLOR,
		FRAG_RGBA,
		FRAG_NOISE3,
		FRAG_NOISE4,
		FRAG_ENVIRONMENT_REFLECT,
		FRAG_ENVIRONMENT_REFRACT,
		FRAG_POINT_LIGHT_RGB,  // vec3 color;
		FRAG_POINT_LIGHT_ADS,  // ambient + diffuse + specular
		FRAG_POINT_LIGHT_TEXTURE,
		FRAG_DEPTH_ORTHOGRAPHIC,
		FRAG_DEPTH_PERSPECTIVE,
		FRAG_NORMAL_MAPPING,
		FRAG_MOTION_BLUR,
		FRAG_MATCAP,                   // normal => semisphere texcoord
		
		COMPUTE_SHADER_START = 500,
		
	};
	
private:
//	static const std::unordered_map<uint32_t, std::string> NAMES;
	static const std::unordered_map<std::string, uint32_t> LOCATIONS;
	
	std::unordered_map<std::string, uint32_t, std::hash<std::string>> shaderMap;

	ShaderFactory();
public:
	ShaderFactory(const ShaderFactory&) = delete;
	ShaderFactory& operator =(const ShaderFactory&) = delete;
	~ShaderFactory();
	
	static uint32_t loadShader(const std::string& path);
	static uint32_t loadShader(Index index);
	static uint32_t loadShader(Shader::Type type, const char* source, size_t length);
	static uint32_t loadShader(Shader::Type type, const std::string& source);

	uint32_t at(const std::string& filename);
};


}  // namespace pea
#endif  // PEA_OPENGL_SHADER_FACTORY_H_
