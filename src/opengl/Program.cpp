#include "opengl/Program.h"

#include <cstdarg>
#include <functional>  // for std::hash
#include <map>

#include "opengl/Shader.h"
#include "opengl/ShaderFactory.h"

#include "util/compiler.h"
#include "util/Log.h"


static const char* TAG = "Program";

using namespace pea;


Program::Program():
		program(NULL_PROGRAM)
{
}

Program::Program(uint32_t program):
		program(program)
{
	assert(program == NULL_PROGRAM || glIsProgram(program));
}

Program::Program(Program&& other):
		program(other.program)
{
	other.program = NULL_PROGRAM;
}

Program& Program::operator =(Program&& other)
{
	glDeleteProgram(this->program);
	this->program = other.program;
	other.program = NULL_PROGRAM;
	return *this;
}

Program::~Program()
{
	glDeleteProgram(program);
//	program = NULL_PROGRAM;
}

uint32_t Program::release()
{
	uint32_t result = program;
	program = NULL_PROGRAM;
	return result;
}

Program::Program(uint32_t vertexShaderIndex, uint32_t fragmentShaderIndex)
{
	uint32_t vertexShader = ShaderFactory::loadShader(static_cast<ShaderFactory::Index>(vertexShaderIndex));
	uint32_t fragmentShader = ShaderFactory::loadShader(static_cast<ShaderFactory::Index>(fragmentShaderIndex));
	assert(vertexShader != 0 && fragmentShader != 0);
	program = Program::createProgram(2, vertexShader, fragmentShader);
	assert(program != 0);
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Program::Program(uint32_t vertexShaderIndex, uint32_t geometryShaderIndex, uint32_t fragmentShaderIndex)
{
	uint32_t vertexShader = ShaderFactory::loadShader(static_cast<ShaderFactory::Index>(vertexShaderIndex));
	uint32_t geometryShader = ShaderFactory::loadShader(static_cast<ShaderFactory::Index>(geometryShaderIndex));
	uint32_t fragmentShader = ShaderFactory::loadShader(static_cast<ShaderFactory::Index>(fragmentShaderIndex));
	assert(vertexShader != 0 && geometryShader != 0 &&fragmentShader != 0);
	program = Program::createProgram(3, vertexShader, geometryShader, fragmentShader);
	assert(program != 0);
	
	glDeleteShader(vertexShader);
	glDeleteShader(geometryShader);
	glDeleteShader(fragmentShader);
}

uint32_t Program::createProgram(int32_t count, /* uint32_t shader */...)
{
	uint32_t program = glCreateProgram();
	std::va_list args;
	va_start(args, count);
	for(int32_t i = 0; i < count; ++i)
	{
		uint32_t shader = va_arg(args, uint32_t);
		assert(shader != 0);
		glAttachShader(program, shader);
	}
	va_end(args);
	glLinkProgram(program);
	
#ifndef NDEBUG
	// check for linking errors
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if(!success)
		showLinkerLog(program);
#endif

	return program;
}

void Program::showLinkerLog(uint32_t program)
{
	bool isProgram = (glIsProgram(program) == GL_TRUE);

	int32_t length;
	if(isProgram)
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
	else
	{
		slog.w(TAG, "Not a program object");
		return;
	}

	char* info = new char[length];
	if(info)
	{
		glGetProgramInfoLog(program, length, nullptr, info);
		slog.e(TAG, "linking program failed:\n\t%s", info);

		delete[] info;
	}
}

void Program::use() const
{
	assert(glIsProgram(program));
	glUseProgram(program);
}

std::string Program::getActiveVariables() const
{
	constexpr char _ = ' ';
	std::ostringstream oss;
	
	auto append = [&oss](bool isUniform, const std::vector<Variable>& variables)
	{
		oss << "//" << _ << variables.size() << _ << "active" << _ << (isUniform? "uniform":"attribute");
		if(variables.size() > 1)
			oss << "s";
		oss << '\n';
		
		for(const Variable& variable: variables)
		{
			// layout(location = 0) uniform mat4 model;  // size=4
			oss << "layout" << '(' << "location =" << std::setw(2) << variable.location << ')' << _
				<< (isUniform? "uniform": "in") << _
				<< GL::glslTypeToString(variable.type) << _
				<< variable.name << ';' << _ << _
				<< "//" << _ << "size=" << variable.size << '\n';
		}
	};
	
	std::vector<Variable> uniforms   = getActiveVariables(program, GL_ACTIVE_UNIFORMS);
	std::vector<Variable> attributes = getActiveVariables(program, GL_ACTIVE_ATTRIBUTES);
	oss << "program name=" << program << '\n';
	append(true, uniforms);
	append(false, attributes);
	return oss.str();
}

int32_t Program::getInputLocation(const char* name) const
{
	return glGetAttribLocation(program, name);
}

int32_t Program::getUniformLocation(const char* name) const
{
	return glGetUniformLocation(program, name);
}

int32_t Program::getOutputLocation(const char* name) const
{
	return glGetFragDataLocation(program, name);
}

std::vector<Program::Variable> Program::getActiveVariables(uint32_t program, GLenum activeType)
{
	assert(activeType == GL_ACTIVE_UNIFORMS || activeType == GL_ACTIVE_ATTRIBUTES);
	std::map<uint32_t, Program::Variable> map;
	
	Program::Variable variable;
	constexpr GLsizei bufferSize = 1024;  // TODO: maximum name length in GLSL?
	GLchar name[bufferSize];  // variable name in GLSL
	GLsizei length;  // name length, excluding the null terminator.
	
	int32_t count;
	glGetProgramiv(program, activeType, &count);
	for(int32_t i = 0; i < count; ++i)
	{
		const GLuint index = static_cast<GLuint>(i);
		int32_t location;
		if(activeType == GL_ACTIVE_UNIFORMS)
		{
			glGetActiveUniform(program, index, bufferSize, &length, &variable.size, &variable.type, name);
			location = glGetUniformLocation(program, name);
		}
		else if(activeType == GL_ACTIVE_ATTRIBUTES)
		{
			glGetActiveAttrib(program, index, bufferSize, &length, &variable.size, &variable.type, name);
			location = glGetAttribLocation(program, name);
		}
		
		variable.location = location;
		variable.name = std::string(name, length);
		map.emplace(location, variable);
	}
	
	// turn std::map to std::vector
	std::vector<Program::Variable> variables;
	variables.reserve(map.size());
	for(auto it = map.begin(); it != map.end(); ++it)
		variables.push_back(it->second);
	return variables;
}

//static bool printed = false;
void Program::setLight(uint32_t program, const char* name, const Light& light)
{
	std::string name_ = std::string(name) + ".";
	// TODO: cache uniforms/attributes location
	auto getMemberLocation = [&program, &name_](const char* member) -> int32_t
	{
		std::string name_member = name_ + member;
		int32_t location = glGetUniformLocation(program, name_member.c_str());
//		if(!printed && location == Shader::INVALID_LOCATION)
//			printf("%s.%s invalid location\n", name_, member);
		
//		assert(location != Shader::INVALID_LOCATION);
		return location;
	};
	
	int32_t ambientLocation = getMemberLocation("ambient");
	int32_t diffuseLocation = getMemberLocation("diffuse");
	int32_t specularLocation = getMemberLocation("specular");
	
	Program::setUniform(ambientLocation, light.ambient);
	Program::setUniform(diffuseLocation, light.diffuse);
	Program::setUniform(specularLocation, light.specular);
	
	switch(light.getType())
	{
	case Light::Type::SPOT:
		{
			int32_t directionLocation = getMemberLocation("direction");
			int32_t cutoffLocation = getMemberLocation("cutoff");
			
			const SpotLight& _light = static_cast<const SpotLight&>(light);
			Program::setUniform(directionLocation, _light.direction);
			
			vec2f cutoff(_light.innerAngle, _light.outerAngle);
			Program::setUniform(cutoffLocation, cutoff);
		}
		[[fallthrough]];
	
	case Light::Type::POINT:
		{
			int32_t positionLocation = getMemberLocation("position");
			int32_t attenuationLocation = getMemberLocation("attenuation");
			int32_t rangeLocation = getMemberLocation("range");
			
			const PointLight& _light = static_cast<const PointLight&>(light);
			Program::setUniform(positionLocation, _light.position);
			if(attenuationLocation != Shader::INVALID_LOCATION)
				Program::setUniform(attenuationLocation, _light.attenuation);
			if(rangeLocation != Shader::INVALID_LOCATION)
				Program::setUniform(rangeLocation, _light.range);
		}
		break;
	
	case Light::Type::DIRECTIONAL:
		{
			int32_t directionLocation = getMemberLocation("direction");
			
			const DirectionalLight& _light = static_cast<const DirectionalLight&>(light);
			setUniform(directionLocation, _light.direction);
		}
		break;
	
	default:
		assert(false);
	}
}

void Program::setMaterial(uint32_t program, const char* name, const Material& material)
{
	std::string name_ = std::string(name) + '.';
	auto getMemberLocation = [&program, &name_](const char* member) -> int32_t
	{
		std::string name_member = name_ + member;
		return glGetUniformLocation(program, name_member.c_str());
	};
	int32_t ambientLocation   = getMemberLocation("ambient");
	int32_t diffuseLocation   = getMemberLocation("diffuse");
	int32_t specularLocation  = getMemberLocation("specular_shininess");
//	int32_t shininessLocation = getMemberLocation("shininess");
//	slog.d(TAG, "location ambient=%d, diffuse=%d, specular=%d", ambientLocation, diffuseLocation, specularLocation);
	assert(ambientLocation >= 0 && diffuseLocation >= 0 && specularLocation >= 0);
	
	vec3f ZERO(0, 0, 0);
	setUniform(ambientLocation, material.illum >= IlluminationMode::COLOR_ON_AND_AMBIENT_ON? material.ambient: ZERO);
	setUniform(diffuseLocation, material.diffuse);
	
	vec4f specular(0, 0, 0, 1);
	if(material.illum >= IlluminationMode::HIGHLIGHT_ON)
	{
		specular.x = material.specular.x;
		specular.y = material.specular.y;
		specular.z = material.specular.z;
		specular.w = material.shininess;
	}
	setUniform(specularLocation, specular);
}

void Program::bindUniformBlock(const char* name, uint32_t binding)
{
	uint32_t blockIndex = glGetUniformBlockIndex(program, name);
	assert(blockIndex != GL_INVALID_INDEX);
	glUniformBlockBinding(program, blockIndex, binding);
}
