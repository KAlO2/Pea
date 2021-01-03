#include "opengl/ShaderFactory.h"

#include <cassert>
#include <cstring>

#include "opengl/GL.h"
#include "opengl/Shader.h"
#include "util/Log.h"
#include "util/utility.h"


static const char* TAG = "ShaderFactory";

using namespace pea;

// builtin shdader has a common header
// GL_ARB_shading_language_420pack  https://www.khronos.org/opengl/wiki/Sampler_(GLSL)
const std::string ShaderFactory::VERSION = R""(
#version 330
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_shading_language_420pack: enable
)"";

// note DON'T add version and extension directives to builtin shaders.
static std::unordered_map<ShaderFactory::Index, const char*> createIndexNameMap()
{
	std::unordered_map<ShaderFactory::Index, const char*> map;
	// vertex shaders
	map[ShaderFactory::VERT_POSITION] =
#include "./shader/position.vert"
	;
	map[ShaderFactory::VERT_M] =
#include "./shader/model.vert"
	;
	map[ShaderFactory::VERT_MVP] =
#include "./shader/modelViewProjection.vert"
	;
	map[ShaderFactory::VERT_M_VP] =
#include "./shader/model_viewProjection.vert"
	;
	map[ShaderFactory::VERT_M_VP2] =
#include "./shader/model_viewProjection2.vert"
	;
	map[ShaderFactory::VERT_M_V_P] =
#include "./shader/model_view_projection.vert"
	;
	map[ShaderFactory::VERT_V_P] =
#include "./shader/view_projection.vert"
	;
	map[ShaderFactory::VERT_VP] =
#include "./shader/viewProjection.vert"
	;
	
	map[ShaderFactory::VERT_TEXCOORD] =
#include "./shader/texcoord.vert"
	;
	map[ShaderFactory::VERT_M_V_P_TEXCOORD] =
#include "./shader/model_view_projection_texcoord.vert"
	;
	map[ShaderFactory::VERT_M_VP_TEXCOORD] =
#include "./shader/model_viewProjection_texcoord.vert"
	;
	map[ShaderFactory::VERT_VP_TEXCOORD] =
#include "./shader/viewProjection_texcoord.vert"
	;
	map[ShaderFactory::VERT_M_VP_TEXCOORD3] =
#include "./shader/model_viewProjection_texcoord3.vert"
	;
	map[ShaderFactory::VERT_M_VP_TEXCOORD_INSTANCE0] =
#include "./shader/model_viewProjection_texcoord_instance0.vert"
	;
	map[ShaderFactory::VERT_M_VP_TEXCOORD_INSTANCE1] =
#include "./shader/model_viewProjection_texcoord_instance1.vert"
	;
	map[ShaderFactory::VERT_M_V_P_COLOR] =
#include "./shader/model_view_projection_color.vert"
	;
	map[ShaderFactory::VERT_M_VP_COLOR] =
#include "./shader/model_viewProjection_color.vert"
	;
	map[ShaderFactory::VERT_M_VP_WEIGHT] =
#include "./shader/model_viewProjection_weight.vert"
	;
	map[ShaderFactory::VERT_M_VP_RGBA] =
#include "./shader/model_viewProjection_rgba.vert"
	;
	map[ShaderFactory::VERT_BILLBOARD] =
#include "./shader/billboard.vert"
	;
//	map[ShaderFactory::VERT_M_V_P_TEXCOORD_COLOR] =
//#include "./shader/model_view_projection_texcoord_color.vert"
//	;

	map[ShaderFactory::VERT_M_VP_HEIGHT_TEXCOORD] =
#include "./shader/model_viewProjection_height_texcoord.vert"
	;
	map[ShaderFactory::VERT_M_V_P_NORMAL] =
#include "./shader/model_view_projection_normal.vert"
	;
	map[ShaderFactory::VERT_M_VP_NORMAL] =
#include "./shader/model_viewProjection_normal.vert"
	;
	map[ShaderFactory::VERT_M_VP_NORMAL2] =
#include "./shader/model_viewProjection_normal2.vert"
	;
	map[ShaderFactory::VERT_M_VP_NORMAL_FLAT] =
#include "./shader/model_viewProjection_normal.flat.vert"
	;
	map[ShaderFactory::VERT_M_VP_NORMAL_TEXCOORD] =
#include "./shader/model_viewProjection_normal_texcoord.vert"
	;
	map[ShaderFactory::VERT_M_VP_SPHERE_NORMAL] =
#include "./shader/model_viewProjection_sphere_normal.vert"
	;
	map[ShaderFactory::VERT_M_VP_SPHERE_TEXCOORD] =
#include "./shader/model_viewProjection_sphere.vert"
	;
	map[ShaderFactory::VERT_TEXTURE_FONT] =
#include "./shader/texture_font.vert"
	;
	map[ShaderFactory::VERT_SKYBOX] =
#include "./shader/skybox.vert"
	;
	
	map[ShaderFactory::VERT_NORMAL_MAPPING] =
#include "./shader/normal_mapping.vert"
	;
	map[ShaderFactory::VERT_MOTION_BLUR] =
#include "./shader/motion_blur.vert"
	;
	map[ShaderFactory::VERT_DEFERRED_COLOR] =
#include "./shader/deferred_color.vert"
	;
	map[ShaderFactory::VERT_GBUFFER] =
#include "./shader/gbuffer.vert"
	;
	map[ShaderFactory::VERT_BONE] =
#include "./shader/bone.vert"
	;
	map[ShaderFactory::VERT_SKINNING2] =
#include "./shader/skinning2.vert"
	;
	map[ShaderFactory::VERT_SKINNING4] =
#include "./shader/skinning4.vert"
	;
	
	
	
	// geometry shaders
	map[ShaderFactory::GEOM_BILLBOARD] =
#include "./shader/billboard.geom"
	;
	map[ShaderFactory::GEOM_NORMAL] =
#include "./shader/normal.geom"
	;
	
	
	// fragment shaders
	map[ShaderFactory::FRAG_TEXTURE_LUMINANCE] =
#include "./shader/texture_luminance.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_RGB] =
#include "./shader/texture_rgb.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_RECT_RGB] =
#include "./shader/textureRect_rgb.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_RGBA] =
#include "./shader/texture_rgba.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_COLOR_ALPHA] =
#include "./shader/texture_color_alpha.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_ALPHA_COLOR] =
#include "./shader/texture_alpha_color.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_RGBA_ALPHA] =
#include "./shader/texture_rgba_alpha.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_FONT] =
#include "./shader/texture_font.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_CUBE] =
#include "./shader/textureCube.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_BLEND] =
#include "./shader/texture_blend.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_3D] =
#include "./shader/texture3d.frag"
	;
	map[ShaderFactory::FRAG_NOISE3] =
#include "./shader/noise3.frag"
	;
	map[ShaderFactory::FRAG_NOISE4] =
#include "./shader/noise4.frag"
	;
	map[ShaderFactory::FRAG_UNIFORM_COLOR] =
#include "./shader/uniformColor.frag"
	;
	map[ShaderFactory::FRAG_UNIFORM_INDEX] =
#include "./shader/uniformIndex.frag"
	;
	map[ShaderFactory::FRAG_LAMBERT] =
#include "./shader/lambert.frag"
	;
	map[ShaderFactory::FRAG_LAMBERT_FLAT] =
#include "./shader/lambert.flat.frag"
	;
	map[ShaderFactory::FRAG_PBR] =
#include "./shader/pbr.frag"
	;
	map[ShaderFactory::FRAG_ENVIRONMENT_REFLECT] =
#include "./shader/environment_reflect.frag"
	;
	map[ShaderFactory::FRAG_ENVIRONMENT_REFRACT] =
#include "./shader/environment_refract.frag"
	;
	map[ShaderFactory::FRAG_POINT_LIGHT_RGB] =
#include "./shader/point_light_color.frag"
	;
	map[ShaderFactory::FRAG_POINT_LIGHT_ADS] =
#include "./shader/point_light_ambient_diffuse_specular.frag"
	;
	map[ShaderFactory::FRAG_POINT_LIGHT_TEXTURE] =
#include "./shader/point_light_texture.frag"
	;
	map[ShaderFactory::FRAG_COLOR] =
#include "./shader/color.frag"
	;
	map[ShaderFactory::FRAG_RGBA] =
#include "./shader/rgba.frag"
	;
	map[ShaderFactory::FRAG_DEPTH_ORTHOGRAPHIC] =
#include "./shader/texture_depth_orthographic.frag"
	;
	map[ShaderFactory::FRAG_DEPTH_PERSPECTIVE] =
#include "./shader/texture_depth_pespective.frag"
	;
	map[ShaderFactory::FRAG_NORMAL_MAPPING] =
#include "./shader/normal_mapping.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_BLUR_HORIZONTAL] =
#include "./shader/texture_blur.horizontal.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_BLUR_VERTICAL] =
#include "./shader/texture_blur.vertical.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_RECT_BLUR_HORIZONTAL] =
#include "./shader/textureRect_blur.horizontal.frag"
	;
	map[ShaderFactory::FRAG_TEXTURE_RECT_BLUR_VERTICAL] =
#include "./shader/textureRect_blur.vertical.frag"
	;
	map[ShaderFactory::FRAG_MOTION_BLUR] =
#include "./shader/motion_blur.frag"
	;
	map[ShaderFactory::FRAG_RADICAL_BLUR] =
#include "./shader/radical_blur.frag"
	;
	map[ShaderFactory::FRAG_MATCAP] =
#include "./shader/matcap.frag"
	;
	map[ShaderFactory::FRAG_DEFERRED_COLOR] =
#include "./shader/deferred_color.frag"
	;
	map[ShaderFactory::FRAG_GBUFFER] =
#include "./shader/gbuffer.frag"
	;
	map[ShaderFactory::FRAG_DEFERRED_SHADING] =
#include "./shader/deferred_shading.frag"
	;
	map[ShaderFactory::FRAG_BONE] =
#include "./shader/bone.frag"
	;
	map[ShaderFactory::FRAG_POSITION_UNIFORM_COLOR_NORMAL_TEXCOORD] =
#include "./shader/position_uniformColor_normal_texcoord.frag"
	;
	
	return map;
}

static const std::unordered_map<ShaderFactory::Index, const char*> builtinShaderMap = createIndexNameMap();

ShaderFactory::~ShaderFactory()
{
	for(auto it = shaderMap.begin(); it != shaderMap.end(); ++it)
		glDeleteShader(it->second);
	shaderMap.clear();
}

uint32_t ShaderFactory::loadShader(Index index)
{
	return loadShader(index, nullptr, 0U);
}

uint32_t ShaderFactory::loadShader(Index index, std::string macros[][2], uint32_t macroCount)
{
	auto it = builtinShaderMap.find(index);
	if(it == builtinShaderMap.end())
	{
		slog.d(TAG, "builtin shader #%u doesn't exist", index);
		return 0;
	}
	
	using Stage = Shader::Stage;
	Stage stage = Stage::UNKNOWN;
	if(index < Index::TESS_CONTROL_SHADER_START)
		stage = Stage::VERTEX;
	else if(index < Index::TESS_EVALUATION_SHADER_START)
		stage = Stage::TESS_CONTROL;
	else if(index < Index::GEOMETRY_SHADER_START)
		stage = Stage::TESS_EVALUATION;
	else if(index < Index::FRAGMENT_SHADER_START)
		stage = Stage::GEOMETRY;
	else if(index < Index::COMPUTE_SHADER_START)
		stage = Stage::FRAGMENT;
	else
		stage = Stage::COMPUTE;

	const char* content = it->second;
	
	const bool hasMacro = macros != nullptr && macroCount > 0;
	size_t length = VERSION.length() + std::strlen(content);
	if(hasMacro)
	{
		// "#define" ' ' key ' ' value '\n'
		for(uint32_t i = 0; i < macroCount; ++i)
			length += 10 + macros[i][0].length() + macros[i][1].length();
	}
	
	std::string source;
	source.reserve(length);
	source.append(VERSION);
	if(hasMacro)
	{
		for(uint32_t i = 0; i < macroCount; ++i)
			source.append("#define").append(1, ' ')
					.append(macros[i][0]).append(1, ' ')
					.append(macros[i][1]).append(1, '\n');
	}
	
	source.append(content);
	assert(source.length() == length);
	return loadShader(stage, source);
}

uint32_t ShaderFactory::loadShader(Shader::Stage stage, const char* source, size_t length)
{
	return Shader::compile(stage, source, length);
}

uint32_t ShaderFactory::loadShader(Shader::Stage stage, const std::string& source)
{
	return Shader::compile(stage, source.c_str(), source.length());
}

uint32_t ShaderFactory::at(const std::string& filename)
{
/*
	filename suffix can be one of the followings values, where they are named after shader type.
		.vert for a vertex shader
		.tesc for a tessellation control shader
		.tese for a tessellation evaluation shader
		.geom for a geometry shader
		.frag for a fragment shader
		.comp for a compute shader
*/
	size_t length = filename.length();
	if(length < std::string("*.type").length())
		return 0;

	const auto it = shaderMap.find(filename);
	if(it != shaderMap.end())
		return it->second;
/*
	const char* p = filename.c_str() + filename.length();
	p -= 5;
	if(*p != '.')
	{
		slog.w(TAG, "filename [%s] suffix doesn't belongs to {'vert', 'tesc', 'tese', 'geom', 'frag', 'comp'}", filename.c_str());
		return 0;
	}

	int type = *reinterpret_cast<const int*>(p + 1);

	constexpr uint32_t VERT = makeFourCC('v', 'e', 'r', 't');
	constexpr uint32_t TESC = makeFourCC('t', 'e', 's', 'c');
	constexpr uint32_t TESE = makeFourCC('t', 'e', 's', 'e');
	constexpr uint32_t GEOM = makeFourCC('g', 'e', 'o', 'm');
	constexpr uint32_t FRAG = makeFourCC('f', 'r', 'a', 'g');
	constexpr uint32_t COMP = makeFourCC('c', 'o', 'm', 'p');

	Shader* shader = nullptr;
	switch(type)
	{
	case VERT: shader = new (std::nothrow) VertexShader();         break;
	case TESC: shader = new (std::nothrow) TessControlShader();    break;
	case TESE: shader = new (std::nothrow) TessEvaluationShader(); break;
	case GEOM: shader = new (std::nothrow) GeometryShader();       break;
	case FRAG: shader = new (std::nothrow) FragmentShader();       break;
	case COMP: shader = new (std::nothrow) ComputeShader();        break;
	default:   assert(false && "unknown shader type");             break;
	}

	shader->loadFromFile(filename);
	bool flag = shader->compile();
	assert(flag);  // fast fail

	uint32_t id = shader->getId();
	shaderMap[filename] = id;

	delete shader;

	return id;
*/
	return 0;
}
