#include "scene/Object.h"

#include <cassert>

#include "opengl/GL.h"
#include "opengl/Program.h"
#include "opengl/Shader.h"

using namespace pea;

Object::Object():
		transform(1.0F),
		program(Program::NULL_PROGRAM),
		light(false)
{
	
}

void Object::setProgram(uint32_t program)
{
	this->program = program;

	if(program != Program::NULL_PROGRAM)
	{
//		assert(glGetUniformLocation(program, "model") == Shader::UNIFORM_MAT_MODEL);
//		assert(glGetUniformLocation(program, "viewProjection") == Shader::UNIFORM_MAT_VIEW_PROJECTION);
//		assert(glGetAttribLocation(program, "position") == Shader::ATTRIBUTE_VEC_POSITION);
	}
}

void Object::render(const mat4f& viewProjection) const
{
	assert(glIsProgram(program));
	Program::use(program);
	
	Program::setUniform(Shader::UNIFORM_MAT_MODEL, transform);
	Program::setUniform(Shader::UNIFORM_MAT_VIEW_PROJECTION, viewProjection);
}

void Object::update(float dt)
{
	(void)dt;
}
