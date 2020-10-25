#include "opengl/QuadRenderer.h"

#include "opengl/GL.h"
#include "opengl/Program.h"
#include "opengl/ShaderFactory.h"

using namespace pea;

QuadRenderer::QuadRenderer():
		vao(0),
		vbo(0),
		texture(nullptr)
{
	
}

QuadRenderer::~QuadRenderer()
{
	texture = nullptr;
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

void QuadRenderer::prepare()
{
/*
	uint32_t program = getProgram();
	if(program != 0)
		return;

	Program colorProgram(ShaderFactory::VERT_TEXCOORD, ShaderFactory::FRAG_TEXTURE_RGBA);

	program = colorProgram.release();
	setProgram(program);
*/
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
}

void QuadRenderer::upload() const
{
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
#if 0
	const float vertices[] =
	{ 
		//       position ,   texcoord,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,  // left top
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // left bottom
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  // right top
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // right bottom
	};
	
	constexpr int32_t stride = 5 * sizeof(float);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(Shader::ATTRIBUTE_VEC_POSITION);
	glVertexAttribPointer(Shader::ATTRIBUTE_VEC_POSITION, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glEnableVertexAttribArray(Shader::ATTRIBUTE_VEC_TEXCOORD);
	glVertexAttribPointer(Shader::ATTRIBUTE_VEC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
#else
	const float vertices[] =
	{ 
		// position
		-1.0f,  1.0f, 0.0f,  // left top
		-1.0f, -1.0f, 0.0f,  // left bottom
		 1.0f,  1.0f, 0.0f,  // right top
		 1.0f, -1.0f, 0.0f,  // right bottom
		// texcoord
		 0.0f, 1.0f,
		 0.0f, 0.0f,
		 1.0f, 1.0f, 
		 1.0f, 0.0f,
	};
	
	constexpr int32_t stride = 0;
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STREAM_DRAW);
	glEnableVertexAttribArray(Shader::ATTRIBUTE_VEC_POSITION);
	glVertexAttribPointer(Shader::ATTRIBUTE_VEC_POSITION, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(Shader::ATTRIBUTE_VEC_TEXCOORD);
	glVertexAttribPointer(Shader::ATTRIBUTE_VEC_TEXCOORD, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(12 * sizeof(float)));
#endif
	glBindVertexArray(0);
}

static constexpr int32_t textureUnit = 0;
void QuadRenderer::setProgram(uint32_t program)
{
	Object::setProgram(program);
}

void QuadRenderer::updateTexcoord(const vec2f quad[4])
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	constexpr size_t offset = 4 * sizeof(vec3f);  // position
	constexpr size_t size   = 4 * sizeof(vec2f);  // texcoord
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, quad);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void QuadRenderer::setTexture(const Texture* texture)
{
//	assert(unit >= 0);
	this->texture = texture;
}

void QuadRenderer::render(const mat4f& viewProjection) const
{
	std::ignore = viewProjection;
	const uint32_t& program = getProgram();
	glUseProgram(program);
/*
	Program::setUniform(Shader::UNIFORM_MAT_MODEL, transform);
	Program::setUniform(Shader::UNIFORM_MAT_VIEW_PROJECTION, viewProjection);
*/
	if(texture)
	{
		glUniform1i(Shader::UNIFORM_TEX_TEXTURE0, textureUnit);
		texture->bind(textureUnit);
	}
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
