#include "opengl/Shader.h"

#include "io/FileSystem.h"
#include "opengl/GL.h"
#include "util/Log.h"


static const char* TAG = "Shader";

using namespace pea;

static const std::pair<int32_t, const char*> map2GL[Shader::TYPE_COUNT] =
{
	{ GL_VERTEX_SHADER, "vertex shader" },
	{ GL_TESS_CONTROL_SHADER, "tessellation control shader" },
	{ GL_TESS_EVALUATION_SHADER, "tessellation evaluation shader" },
	{ GL_GEOMETRY_SHADER, "geometry shader" },
	{ GL_FRAGMENT_SHADER, "fragment shader" },
	{ GL_COMPUTE_SHADER, "compute shader" },
};

Shader::Shader(Type type):
		type(type),
		shader_id(0)
{

}

Shader::Shader(const Shader& other):
		type(other.type),
		shader_id(other.shader_id),
		tag(other.tag),
		source()
{
	// strip filename and source
}

Shader& Shader::operator =(const Shader& other)
{
	if(this != &other)
	{
		type = other.type;
		shader_id = other.shader_id;
		tag = other.tag;
		source = std::string();  // source.clear(); source.shrink_to_fit();
	}
	return *this;
}

bool Shader::loadFromFile(const std::string& filename)
{
	this->tag = filename;
	this->source = FileSystem::load(filename);
	return !source.empty();
}

bool Shader::loadFromMemory(const std::string& tag, const std::string& source)
{
//	std::ostringstream handle;
//	handle << "addr@" << reinterpret_cast<uintptr_t>(source.data()) << "len" << source.length();
//	this->tag = handle.str();
	this->tag = tag;
	this->source = source;
	return true;
}
/*
bool Shader::compile()
{
	assert(0 <= type && type < Shader::TYPE_COUNT);
	shader_id = glCreateShader(map2GL[type].first);
	const char* src = source.c_str();
	const int32_t length = source.length();

	glShaderSource(shader_id, 1, &src, &length);
	glCompileShader(shader_id);

	slog.v(TAG, "create %s, id: %d", map2GL[type].second, shader_id);

	GLint compiled = GL_FALSE;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);

	if(compiled != GL_TRUE)
	{
		showCompilerLog();
		return false;
	}

	return true;
}
*/
uint32_t Shader::compile(Type type, const char* source, int32_t length)
{
	return compile(type, 1, &source, &length);
}

uint32_t Shader::compile(Type type, uint32_t count, const char** source, int32_t* length)
{
	assert(0 <= type && type < Shader::TYPE_COUNT);
	GLenum shaderType = map2GL[type].first;
	uint32_t shader = glCreateShader(shaderType);
	constexpr uint32_t INVALID_SHADER = 0;
	if(shader == INVALID_SHADER)
	{
		slog.e(TAG, "create shader of type=%s (%d)", map2GL[type].second, shaderType);
		GL::checkError();
		return INVALID_SHADER;
	}
	
	glShaderSource(shader, count, source, length);
	glCompileShader(shader);
#ifndef NDEBUG
	GLint compiled = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if(compiled != GL_TRUE)
	{
		showCompilerLog(map2GL[type].second, shader);
		return INVALID_SHADER;
	}
#endif
	return shader;
}

void Shader::showCompilerLog(const char* tag, uint32_t shader)
{
	bool isShader = (glIsShader(shader) == GL_TRUE);

	int32_t length;
	if(isShader)
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
	else
	{
		slog.w(TAG, "Not a shader object");
		return;
	}

	char* info = new (std::nothrow) char[length];
	if(info)
	{
		glGetShaderInfoLog(shader, length, nullptr, info);
		slog.e(TAG, "compiling %s failed:\n%s", tag, info);

		delete[] info;
	}
}
/*
void GeometryShader::setProgramParameter(uint32_t program_id)
{
	assert(glIsProgram(program_id) == GL_TRUE);

	glProgramParameter(program_id, GL_GEOMETRY_INPUT_TYPE, GL::glCast(input));
	glProgramParameter(program_id, GL_GEOMETRY_OUTPUT_TYPE, GL::glCast(output));
	glProgramParameter(program_id, GL_GEOMETRY_VERTICES_OUT, out_vertices_size);
}
*/

