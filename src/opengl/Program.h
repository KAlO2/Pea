#ifndef PEA_OPENGL_PROGRAM_H_
#define PEA_OPENGL_PROGRAM_H_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

// vector and matrix
#include "opengl/GL.h"
#include "math/mat2.h"
#include "math/mat3.h"
#include "math/mat3x4.h"
#include "math/mat4.h"
#include "graphics/Color.h"
#include "scene/Material.h"
#include "scene/Light.h"

namespace pea {

// shader can be reuse across different programs.
class Program final
{
public:
	static constexpr uint32_t NULL_PROGRAM = 0;
	static constexpr int32_t INVALID_LOCATION = -1;  // attribute and uniform
/*
	// S_* for shader, A_* for attribute, U_* for uniform.
	static const char* ATTRIBUTE_POSITION;
	static const char* ATTRIBUTE_COLOR;
	static const char* ATTRIBUTE_NORMAL;
	static const char* ATTRIBUTE_TANGENT;
	static const char* ATTRIBUTE_BINORMAL;
	static const char* ATTRIBUTE_TEXCOORD0;
	static const char* ATTRIBUTE_TEXCOORD1;
	static const char* ATTRIBUTE_TEXCOORD2;
	static const char* ATTRIBUTE_TEXCOORD3;

	static const char* ATTRIBUTE_TEXT_COLOR;

	static const char* UNIFORM_MODEL_MATRIX;
	static const char* UNIFORM_VIEW_MATRIX;
	static const char* UNIFORM_PROJECTION_MATRIX;
	static const char* UNIFORM_TIME;

	static const char* UNIFORM_TEXTURE_FONT;
*/
	class Variable;

private:
//	uint32_t shaders[Shader::TYPE_COUNT];
//	std::vector<uint32_t> shaders;
	uint32_t program;  // program name

private:
	// Specify whether to transpose the matrix as the values are loaded.
//	static const bool TRANSPOSE = COLUMN_MAJOR;

	static void showLinkerLog(uint32_t program);

public:
	Program();
	
	/**
	 * @param[in] program a valid program created by glCreateProgram(). After constructed, it will
	 *            take in charge, namely, you don't have to call glDeleteProgram() explicitly.
	 */
	explicit Program(uint32_t program);
	~Program();
	
	/**
	 * behaviour like std::unique_ptr<T>#release(), returns the program object and releases the 
	 * ownership. You must call #glDeleteProgram() afterwards.
	 */
	[[nodiscard]] uint32_t release();
	
	/**
	 * @param[in] vertexShaderIndex   builtin vertex shader defined in ShaderFactory::VERT_XXX
	 * @param[in] fragmentShaderIndex builtin fragment shader defined in ShaderFactory::FRAG_XXX
	 */
	Program(uint32_t vertexShaderIndex, uint32_t fragmentShaderIndex);
	
	/**
	 * @param[in] vertexShaderIndex   builtin vertex shader defined in ShaderFactory::VERT_XXX
	 * @param[in] geometryShaderIndex builtin geometry shader defined in ShaderFactory::GEOM_XXX
	 * @param[in] fragmentShaderIndex builtin fragment shader defined in ShaderFactory::FRAG_XXX
	 */
	Program(uint32_t vertexShaderIndex, uint32_t geometryShaderIndex, uint32_t fragmentShaderIndex);
	
	
	Program(const Program& other) = delete;
	Program& operator =(const Program& other) = delete;
	
	const uint32_t& getName() const;
	
	void use() const;

	
	void printVariables() const;

	int32_t getAttributeLocation(const char* name) const;
	int32_t getUniformLocation(const char* name) const;

	int32_t getAttributeLocation(const std::string& name) const;
	int32_t getUniformLocation(const std::string& name) const;

	void setLight(const char* name, const Light& light) const;
	
	void setMaterial(const char* name, const Material& material) const;
	
	void bindUniformBlock(const char* name, uint32_t index);
	
public:
	static uint32_t createProgram(int32_t count, /* uint32_t shader */...);
	
	static void use(const uint32_t& program);
	
	static std::unordered_map<std::string, Variable> getActiveUniforms(const uint32_t& program);
	static std::unordered_map<std::string, Variable> getActiveAttributes(const uint32_t& program);
	
	static void setLight(uint32_t program, const char* name, const Light& light);
	
	static void setMaterial(uint32_t program, const char* name, const Material& material);
	
	static void setTexture(uint32_t location, uint32_t textureUnit);
	
	/**
	 * @{group uniform
	 */
	static void setUniform(int32_t location, const bool& value);
	static void setUniform(int32_t location, const int32_t& value);
	static void setUniform(int32_t location, const uint32_t& value);
//	static void setUniform(int32_t location, const vec4b& value);

	static void setUniform(int32_t location, const float& value);
	static void setUniform(int32_t location, const vec2f& value);
	static void setUniform(int32_t location, const vec3f& value);
	static void setUniform(int32_t location, const vec4f& value);
	static void setUniform(int32_t location, const mat2f& value);
	static void setUniform(int32_t location, const mat3f& value);
	static void setUniform(int32_t location, const mat4f& value);

	/** } */
	
	/**
	 * For affine transform, the last row of column_major matrix is always (0, 0, 0, 1). It's fully
	 * possible to use mat3x4. While in Perspective transform, the last row is (0, 0, 01, 0).
	 */
	static void setUniform(int32_t location, const mat3x4f& value);
	
//	static static void bindUniformBlock(uint32_t program, const char* name, uint32_t index);
};

inline const uint32_t& Program::getName() const { return program; }

inline void Program::use(const uint32_t& program) { glUseProgram(program); }

inline void Program::setLight(const char* name, const Light& light) const { return setLight(program, name, light); }

inline void Program::setMaterial(const char* name, const Material& material) const { return setMaterial(program, name, material); }

inline void Program::setUniform(int32_t location, const bool& value)     { glUniform1i(location, static_cast<int32_t>(value)); }
inline void Program::setUniform(int32_t location, const int32_t& value)  { glUniform1i(location, value); }
inline void Program::setUniform(int32_t location, const uint32_t& value) { glUniform1ui(location, value); }
//void Program::setUniform(int32_t location, const vec4b& value)    { glUniform4b(location, value.x, value.y, value.z, value.w); }

inline void Program::setTexture(uint32_t location, uint32_t textureUnit) { glUniform1i(location, textureUnit); }

inline void Program::setUniform(int32_t location, const float& value) { glUniform1f(location, value); }
inline void Program::setUniform(int32_t location, const vec2f& value) { glUniform2f(location, value.x, value.y); }
inline void Program::setUniform(int32_t location, const vec3f& value) { glUniform3f(location, value.x, value.y, value.z); }
inline void Program::setUniform(int32_t location, const vec4f& value) { glUniform4f(location, value.x, value.y, value.z, value.w); }

// glUniformMatrix use column major matrix
inline void Program::setUniform(int32_t location, const mat2f& value) { glUniformMatrix2fv(location, 1, !COLUMN_MAJOR, value.data()); }
inline void Program::setUniform(int32_t location, const mat3f& value) { glUniformMatrix3fv(location, 1, !COLUMN_MAJOR, value.data()); }
inline void Program::setUniform(int32_t location, const mat3x4f& value) { glUniformMatrix3x4fv(location, 1, !COLUMN_MAJOR, value.data()); }
inline void Program::setUniform(int32_t location, const mat4f& value) { glUniformMatrix4fv(location, 1, !COLUMN_MAJOR, value.data()); }

inline int32_t Program::getAttributeLocation(const std::string& name) const
{
	return getAttributeLocation(name.c_str());
}

inline int32_t Program::getUniformLocation(const std::string& name) const
{
	return getUniformLocation(name.c_str());
}

class Program::Variable
{
public:
	uint32_t location;
	int32_t size;   // size of the variable
	uint32_t type;  // type of the variable (float, vec3 or mat4, etc)
};

}  // namespace pea
#endif  // PEA_OPENGL_PROGRAM_H_
