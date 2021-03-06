#ifndef PEA_OPENGL_SHADER_H_
#define PEA_OPENGL_SHADER_H_

#include <string>

namespace pea {

class Shader final
{
public:
	// https://www.opengl.org/wiki/Rendering_Pipeline_Overview
	enum Stage: int32_t
	{
		UNKNOWN = -1,

		VERTEX = 0,
		TESS_CONTROL,
		TESS_EVALUATION,
		GEOMETRY,
		FRAGMENT,
		COMPUTE,  // available only if GL_VERSION >= 4.3

		COUNT,
	};

	enum Location: int32_t
	{
		// Note that attribute and uniform are unrelated, so they can use the samle location
		// in/out mat4 take 4 continual locations, uniform mat4 take 1 location.
		INVALID_LOCATION = -1,  // glGetAttribLocation / glGetUniformLocation
		
		ATTRIBUTE_VEC_VERTEX   = 0,  // vec4 vertex = vec2 position + vec2 texcoord;
		ATTRIBUTE_VEC_POSITION = 0,
		ATTRIBUTE_VEC_COLOR    = 1,  // vertex color
		ATTRIBUTE_VEC_NORMAL   = 2,
		ATTRIBUTE_VEC_TEXCOORD = 3,
		ATTRIBUTE_VEC_TANGENT  = 4,
		ATTRIBUTE_VEC_BITANGENT= 5,
		ATTRIBUTE_VEC_SIZE     = 6,  // particle size
		ATTRIBUTE_FLT_WEIGHT   = 7,  // float weight
		
		ATTRIBUTE_VEC_FACE_NORMAL = 8,
		ATTRIBUTE_INT_INDEX    = 10,  // at last
		ATTRIBUTE_VEC_INSTANCE = 12,  // vec4 = vec3 translation + float scale
		ATTRIBUTE_MAT_INSTANCE = 12,  // mat4 translation, rotation, scale
		
	//	UNIFORM_BEGIN_LOCATION
		// Note that a mat4 consumes 4 consecutive locations
		UNIFORM_VEC_POSITION      = 0,  // model position, no rotation or scaling.
		UNIFORM_MAT_MODEL         = 0,
		UNIFORM_MAT_MODEL_VIEW    = 0,
		UNIFORM_MAT_MODEL_VIEW_PROJECTION = 0,
		UNIFORM_MAT_VIEW          = 1,
		UNIFORM_MAT_VIEW_PROJECTION = 1,
		UNIFORM_MAT_PROJECTION    = 2,
		UNIFORM_MAT_LIGHT_VIEW_PROJECTION = 2,
		
		UNIFORM_VEC_COLOR         = 3,  // vec3 color
		UNIFORM_VEC_UNIFORM_COLOR = 3,  // vec4 uniformColor
		UNIFORM_VEC_AMBIENT_COLOR = 3,  // vec3 ambientColor
		UNIFORM_VEC_TEXT_COLOR    = 4,
		UNIFORM_VEC_OBJECT_COLOR  = 4,
		UNIFORM_VEC_LIGHT_POSITION= 5,
		UNIFORM_VEC_LIGHT_DIRECTION = 6,
		UNIFORM_VEC_LIGHT_COLOR   = 7,
		UNIFORM_VEC_VIEW_POSITION = 8,
		UNIFORM_VEC_LAST_POSITION = 9,
		UNIFORM_VEC_SPECULAR_COLOR= 10,  // vec3 color + float shininess
		
		// float alpha, ratio, factor, length, scale, step, time, weight;
		UNIFORM_VEC_PARAMETER     = 10,
		UNIFORM_VEC_CENTER        = 11,  // vec2 center
		UNIFORM_VEC_SCALE         = 11,
		UNIFORM_VEC_SIZE          = 11,  // vec2 size
		UNIFORM_VEC_STEP          = 11,  // vec2 step
		UNIFORM_VEC_DELTA         = 11,  // vec2 delta
		UNIFORM_VEC_ATTENNUATION  = 11,
		UNIFORM_FLT_ALPHA         = 11,
		UNIFORM_FLT_DEPTH         = 11,
		UNIFORM_FLT_FACTOR        = 11,
		UNIFORM_FLT_LENGTH        = 11,
		UNIFORM_FLT_RADIUS        = 11,
		UNIFORM_FLT_RATIO         = 11,
		UNIFORM_FLT_ETA           = 11,  // float ior;  // index of reflection
		UNIFORM_FLT_SCALE         = 11,
		UNIFORM_FLT_STEP          = 11,
		UNIFORM_FLT_WEIGHT        = 11,
		
		// int size, index;
		UNIFORM_INT_PARAMETER     = 12,
		UNIFORM_INT_SIZE          = 13,
		UNIFORM_INT_INDEX         = 13,  // int index;
		
		UNIFORM_UINT_INDEX        = 14,  // uint index;
		
		UNIFORM_FLT_TIME          = 15,  // float time;
		
		UNIFORM_TEX_TEXTURE0      = 16,
		UNIFORM_TEX_TEXTURE1,
		UNIFORM_TEX_TEXTURE2,
		UNIFORM_TEX_TEXTURE3,
		UNIFORM_TEX_TEXTURE4,
		UNIFORM_TEX_TEXTURE5,
		UNIFORM_TEX_TEXTURE6,
		UNIFORM_TEX_TEXTURE7,

/*
		
		UNIFORM_TEX_EMISSIVE,
		UNIFORM_TEX_DIFFUSE,
		
		UNIFORM_MTL_SHININESS,
		UNIFORM_MTL_ALPHA,
		
		UNIFORM_MIX_AMBIENT,
		UNIFORM_MIX_DIFFUSE,
		UNIFORM_MIX_SPECULAR,
*/
	};
	
protected:
	Stage stage;
	uint32_t id;  //< 0 if an error occurs creating the shader object

	std::string tag;  //< filename if loaded from file, else hash(source)
	std::string source;

protected:
	Shader(Stage stage);
	
	static void showCompilerLog(const char* tag, uint32_t shader);

public:
	~Shader() = default;
	Shader(const Shader& other);
	Shader& operator=(const Shader& other);

	bool loadFromFile(const std::string& filename);
	bool loadFromMemory(const std::string& tag, const std::string& source);

	static uint32_t compile(Stage stage, const std::string& source);
	
	/**
	 * Compile shader source in one file.
	 */
	static uint32_t compile(Stage stage, const char* source, int32_t length);
	
	/**
	 * Compile shader source with several segments. You seperate shader source into version, macros,
	 * variables, common utility functions, main function segments.
	 */
	static uint32_t compile(Stage stage, uint32_t count, const char** source, int32_t* length);
	
	Stage getStage() const;
	
	uint32_t getId() const;
};

inline Shader::Stage Shader::getStage() const { return stage; }
inline uint32_t      Shader::getId()    const { return id;    }

}  // namespace pea
#endif  // PEA_OPENGL_SHADER_H_
