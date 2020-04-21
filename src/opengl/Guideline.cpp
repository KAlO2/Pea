#include "opengl/Guideline.h"

#include <cassert>
#include <vector>

#include "opengl/GL.h"
#include "opengl/Program.h"
#include "opengl/ShaderFactory.h"

using namespace pea;

/**
 * @param[in] length semi X/Y/Z axis' length
 */
std::vector<vec3f> createAxisVertices(float length)
{
	assert(length > 0);
	float headLength = length / 10;
	float angle = M_PI / 6;
	float dx = headLength * std::cos(angle);
	float dy = headLength * std::sin(angle);
	
	std::vector<vec3f> vertices
	{
		// X axis
		vec3f(-length, 0, 0),  // left
		vec3f(+length, 0, 0),  // right
		
		vec3f(length - dx, +dy, 0),
		vec3f(length, 0, 0),
		vec3f(length, 0, 0),
		vec3f(length - dx, -dy, 0),
		
		// Y axis
		vec3f(0, -length, 0),  // bottom
		vec3f(0, +length, 0),  // top
		
		vec3f(-dy, length - dx, 0),
		vec3f(0, length, 0),
		vec3f(0, length, 0),
		vec3f(+dy, length - dx, 0),
		
		// Z axis
		vec3f(0, 0, -length),  // bottom
		vec3f(0, 0, +length),  // top
		
		vec3f(-dy, 0, length - dx),
		vec3f(0, 0, length),
		vec3f(0, 0, length),
		vec3f(+dy, 0, length - dx),
	};
	
	return vertices;
}

Guideline::Guideline():
		gridColor(74.0 / 255, 74.0 / 255, 74.0 / 255, 1.0),
		vao(0),
		vbo(0)
{
	setProgram(0);
}

Guideline::~Guideline()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteProgram(getProgram());
}

// glBindVertexArray(vao);
static constexpr int8_t N = 8;
static constexpr uint8_t vertexCount = (2 * N + 1) * 4 + 3 * 2 + 3 * 2;
void Guideline::uploadVertexData() const
{
	constexpr float EXTEND = static_cast<float>(N);
	std::vector<vec3f> vertices;
	vertices.reserve(vertexCount);
	
	// grid data
	for(int8_t i = -N; i <= N; ++i)
	{
		float x = static_cast<float>(i);
		vertices.emplace_back(-EXTEND, x, 0);
		vertices.emplace_back(+EXTEND, x, 0);

		vertices.emplace_back(x, -EXTEND, 0);
		vertices.emplace_back(x, +EXTEND, 0);
	}
	
	// world axis
	vertices.emplace_back(-EXTEND, 0, 0);
	vertices.emplace_back(+EXTEND, 0, 0);
	vertices.emplace_back(0, -EXTEND, 0);
	vertices.emplace_back(0, +EXTEND, 0);
	vertices.emplace_back(0, 0, -EXTEND);
	vertices.emplace_back(0, 0, +EXTEND);
	
	// local axis, dynamically calculated
//	for(uint8_t i = 0; i < 6; ++i)
//		vertices.emplace_back(0, 0, 0);
//	const float infinity = std::numeric_limits<float>::infinity();
	vertices.emplace_back(-INFINITY, 0, 0);
	vertices.emplace_back(+INFINITY, 0, 0);
	vertices.emplace_back(0, -INFINITY, 0);
	vertices.emplace_back(0, +INFINITY, 0);
	vertices.emplace_back(0, 0, -INFINITY);
	vertices.emplace_back(0, 0, +INFINITY);
	
	assert(vertices.size() == vertexCount);
	
	assert(vao != 0);
	glBindVertexArray(vao);
	GL::bindVertexBuffer(vbo, Shader::ATTRIBUTE_VEC_POSITION, vertices);
}

void Guideline::updateVertexData(Guideline::Flag flag, const vec3f* positions)
{
	// x86 has builtin instructions for first/last non zero bit.
	int32_t start = 0, end = 3;
	for(uint32_t bit = Flag::LOCAL_X; bit < Flag::LOCAL_Z; bit <<= 1U, ++start)
		if((flag & bit) != 0)
			break;
	
	for(uint32_t bit = Flag::LOCAL_Z; bit > (Flag::LOCAL_X << start); bit >>= 1U, --end)
		if((flag & bit) != 0)
			break;
	
	if(start >= end)
		return;
	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	int32_t offset = (vertexCount - 6 + start * 2) * sizeof(vec3f);
	int32_t size = (end - start) * 2 * sizeof(vec3f);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, positions);
}

void Guideline::setFlag(Guideline::Flag flag)
{
	this->flag = flag;
}

Guideline::Flag Guideline::getFlag() const
{
	return flag;
}

void Guideline::addFlag(Guideline::Flag flag)
{
//	this->flag |= flag;
	this->flag = static_cast<Guideline::Flag>(this->flag | flag);
}

void Guideline::clearFlag(Guideline::Flag flag)
{
//	this->flag &= ~flag;
	this->flag = static_cast<Guideline::Flag>(this->flag & ~flag);
}

void Guideline::prepare()
{
	uint32_t program = getProgram();
	if(program != 0)
		return;

	Program colorProgram(ShaderFactory::VERT_VP, ShaderFactory::FRAG_UNIFORM_COLOR);
/*
	assert(colorProgram.getUniformLocation("viewProjection") == Shader::UNIFORM_MAT_VIEW_PROJECTION);
	assert(colorProgram.getAttributeLocation("position") == Shader::ATTRIBUTE_VEC_POSITION);
	assert(colorProgram.getUniformLocation("uniformColor") == Shader::UNIFORM_VEC_UNIFORM_COLOR);
	colorProgram.listVariables();
*/
	program = colorProgram.release();
	setProgram(program);
	
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
}

void Guideline::render(const mat4f& viewProjection) const
{
	const uint32_t& program = getProgram();
	assert(program != 0);
	glUseProgram(program);

	Program::setUniform(Shader::UNIFORM_MAT_VIEW_PROJECTION, viewProjection);
	
	uint32_t gridCount = (2 * N + 1) * 4;
//	glDrawArrays(GL_LINES, 0, gridCount + 2 * 3);
	glDepthFunc(GL_LEQUAL);  // <= makes colored axis overwrite center grid line.
	
	glBindVertexArray(vao);
	if((flag & GRID) != 0)
	{
		Program::setUniform(Shader::UNIFORM_VEC_UNIFORM_COLOR, gridColor);
		glDrawArrays(GL_LINES, 0, gridCount);
	}

	if((flag & (WORLD_X | LOCAL_X)) != 0)
	{
		Program::setUniform(Shader::UNIFORM_VEC_UNIFORM_COLOR, vec4f(1.0, 0.0, 0.0, 1.0));  // red
		if((flag & WORLD_X) != 0)
			glDrawArrays(GL_LINES, gridCount, 2);
		if((flag & LOCAL_X) != 0)
			glDrawArrays(GL_LINES, vertexCount - 6, 2);
	}
	if((flag & (WORLD_Y | LOCAL_Y)) != 0)
	{
		Program::setUniform(Shader::UNIFORM_VEC_UNIFORM_COLOR, vec4f(0.0, 1.0, 0.0, 1.0));  // red
		if((flag & WORLD_Y) != 0)
			glDrawArrays(GL_LINES, gridCount + 2, 2);
		if((flag & LOCAL_Y) != 0)
			glDrawArrays(GL_LINES, vertexCount - 4, 2);
	}
	if((flag & (WORLD_Z | LOCAL_Z)) != 0)
	{
		Program::setUniform(Shader::UNIFORM_VEC_UNIFORM_COLOR, vec4f(0.0, 0.0, 1.0, 1.0));  // blue
		if((flag & WORLD_Z) != 0)
			glDrawArrays(GL_LINES, gridCount + 4, 2);
		if((flag & LOCAL_Z) != 0)
			glDrawArrays(GL_LINES, vertexCount - 2, 2);
	}
	glDepthFunc(GL_LESS);
}
