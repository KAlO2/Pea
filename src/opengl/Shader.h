#ifndef PEA_OPENGL_SHADER_H_
#define PEA_OPENGL_SHADER_H_

#include <string>

namespace pea {

class Shader final
{
public:
	// https://www.opengl.org/wiki/Rendering_Pipeline_Overview
	enum Type
	{
		UNKNOWN_SHADER = -1,

		VERTEX_SHADER = 0,
		TESS_CONTROL_SHADER,
		TESS_EVALUATION_SHADER,
		GEOMETRY_SHADER,
		FRAGMENT_SHADER,
		COMPUTE_SHADER,  // available only if GL_VERSION >= 4.3

		TYPE_COUNT,
	};

	enum Location: int32_t
	{
		// Note that attribute and uniform are unrelated, so they can use the samle location
		INVALID_LOCATION = -1,  // glGetAttribLocation / glGetUniformLocation
		
		ATTRIBUTE_VEC_VERTEX   = 0,  // vec4 vertex = vec2 position + vec2 texcoord;
		ATTRIBUTE_VEC_POSITION = 0,
		ATTRIBUTE_VEC_COLOR    = 1,  // vertex color
		ATTRIBUTE_VEC_NORMAL   = 2,
		ATTRIBUTE_VEC_TEXCOORD = 3,
		ATTRIBUTE_VEC_TANGENT  = 4,
		ATTRIBUTE_VEC_BITANGENT= 5,
		ATTRIBUTE_VEC_INSTANCE = 6,  // vec4 = vec3 translation + float scale
		ATTRIBUTE_MAT_INSTANCE = 6,  // mat4 translation, rotation, scale
		ATTRIBUTE_VEC_SIZE     = 7,  // particle size
		ATTRIBUTE_INT_INDEX    =10,  // at last
//		ATTRIBUTE_VEC_TEXCOORD0= 8,
//		ATTRIBUTE_VEC_TEXCOORD1,
		
	//	UNIFORM_BEGIN_LOCATION
		// Note that a mat4 consumes 4 consecutive locations
		UNIFORM_VEC_POSITION      = 0,  // model position, no rotation or scaling.
		UNIFORM_MAT_MODEL         = 0,
		UNIFORM_MAT_MODEL_VIEW    = 0,
		UNIFORM_MAT_MODEL_VIEW_PROJECTION = 0,
		UNIFORM_MAT_VIEW          = 1 << 2,
		UNIFORM_MAT_VIEW_PROJECTION = 1 << 2,
		UNIFORM_MAT_PROJECTION    = 2 << 2,
		UNIFORM_MAT_LIGHT_VIEW_PROJECTION = 2 << 2,
		
		UNIFORM_VEC_UNIFORM_COLOR = 13,  // vec4 uniformColor
		UNIFORM_VEC_TEXT_COLOR    = 13,
		UNIFORM_VEC_VIEW_POSITION = 14,
		UNIFORM_VEC_LIGHT_POSITION= 15,
		UNIFORM_VEC_LIGHT_COLOR   = 16,
		UNIFORM_VEC_SCALE         = 17,
		
		UNIFORM_FLT_RATIO         = 17,
		UNIFORM_FLT_ALPHA         = 17,
		UNIFORM_FLT_LENGTH        = 17,
		UNIFORM_FLT_SCALE         = 17,
		UNIFORM_FLT_STEP          = 17,
		
		UNIFORM_INT_SIZE          = 18,
		
		UNIFORM_TEX_TEXTURE0 = 20,
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
	Type type;
	uint32_t shader_id;  //< 0 if an error occurs creating the shader object

	std::string tag;  //< filename if loaded from file, else hash(source)
	std::string source;

protected:
	Shader(Type type);
	
	static void showCompilerLog(const char* tag, uint32_t shader);

public:
	~Shader() = default;
	Shader(const Shader& other);
	Shader& operator=(const Shader& other);

	bool loadFromFile(const std::string& filename);
	bool loadFromMemory(const std::string& tag, const std::string& source);

	/**
	 * Compile shader source in one file.
	 */
	static uint32_t compile(Type type, const char* source, int32_t length);
	
	/**
	 * Compile shader source with several segments. You seperate shader source into version, macros,
	 * variables, common utility functions, main function segments.
	 */
	static uint32_t compile(Type type, uint32_t count, const char** source, int32_t* length);
	
//	bool compile(const std::string& source);
//	bool compile(Type type, const std::string& filename);
	

	inline Type     getType() const { return type; }
	inline uint32_t getId() const   { return shader_id;  }

//	GLint getAttribute(const std::string& name) const { return getAttribute(name.c_str()); }
//	GLint getUniform(const std::string& name) const { return getUniform(name.c_str()); }


};

/*
class VertexShader: public Shader
{
public:
	VertexShader(): Shader(Type::VERTEX_SHADER) {}
};

class TessControlShader: public Shader
{
public:
	TessControlShader(): Shader(Type::TESS_CONTROL_SHADER) {}
};

class TessEvaluationShader: public Shader
{
public:
	TessEvaluationShader(): Shader(Type::TESS_EVALUATION_SHADER) {}
};

class FragmentShader: public Shader
{
public:
	FragmentShader(): Shader(Type::FRAGMENT_SHADER) {}
};

class GeometryShader: public Shader
{
private:
	Mesh::Primitive input;
	Mesh::Primitive output;
	int32_t out_vertices_size;
public:
	GeometryShader()
	:Shader(Type::GEOMETRY_SHADER)
	,input(Mesh::Primitive::TRIANGLES)
	,output(Mesh::Primitive::TRIANGLE_STRIP)
	,out_vertices_size(3)
	{}

	void setProgramParameter(uint32_t program_id);

};

class ComputeShader: public Shader
{
public:
	ComputeShader(): Shader(Type::COMPUTE_SHADER) {}
};
*/
}  // namespace pea
#endif  // PEA_OPENGL_SHADER_H_
