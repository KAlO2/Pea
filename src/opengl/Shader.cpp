#include "opengl/Shader.h"

#include "io/FileSystem.h"
#include "opengl/GL.h"
#include "util/Log.h"


static const char* TAG = "Shader";

using namespace pea;

static const std::pair<int32_t, const char*> map2GL[Shader::Stage::COUNT] =
{
	{ GL_VERTEX_SHADER, "vertex shader" },
	{ GL_TESS_CONTROL_SHADER, "tessellation control shader" },
	{ GL_TESS_EVALUATION_SHADER, "tessellation evaluation shader" },
	{ GL_GEOMETRY_SHADER, "geometry shader" },
	{ GL_FRAGMENT_SHADER, "fragment shader" },
	{ GL_COMPUTE_SHADER, "compute shader" },
};

Shader::Shader(Stage stage):
		stage(stage),
		id(0)
{

}

Shader::Shader(const Shader& other):
		stage(other.stage),
		id(other.id),
		tag(other.tag),
		source()
{
	// strip filename and source
}

Shader& Shader::operator =(const Shader& other)
{
	if(this != &other)
	{
		stage  = other.stage;
		id     = other.id;
		tag    = other.tag;
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

uint32_t Shader::compile(Stage stage, const std::string& source)
{
	return compile(stage, source.data(), source.length());
}

uint32_t Shader::compile(Stage stage, const char* source, int32_t length)
{
	return compile(stage, 1, &source, &length);
}

uint32_t Shader::compile(Stage stage, uint32_t count, const char** source, int32_t* length)
{
	assert(0 <= stage && stage < Stage::COUNT);
	GLenum shaderType = map2GL[stage].first;
	uint32_t shader = glCreateShader(shaderType);
	constexpr uint32_t INVALID_SHADER = 0;
	if(shader == INVALID_SHADER)
	{
		slog.e(TAG, "create shader of type=%s (%d)", map2GL[stage].second, shaderType);
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
		showCompilerLog(map2GL[stage].second, shader);
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
