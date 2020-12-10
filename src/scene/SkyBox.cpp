#include "scene/SkyBox.h"

#include "geometry/Cube.h"
#include "opengl/GL.h"
#include "opengl/Program.h"
#include "opengl/Shader.h"
#include "opengl/Texture.h"
#include "scene/Camera.h"
#include "util/utility.h"

using namespace pea;


SkyBox::SkyBox():
		program(0),
		texture(nullptr),
		vao(0),
		vbo{0, 0}
{
}

SkyBox::~SkyBox()
{
	glDeleteProgram(program);
	texture = nullptr;
	glDeleteBuffers(sizeofArray(vbo), vbo);
	glDeleteVertexArrays(1, &vao);
}

void SkyBox::setTexture(Texture* texture)
{
	assert(texture == nullptr || texture->getTarget() == GL_TEXTURE_CUBE_MAP);
	this->texture = texture;
}

void SkyBox::prepare()
{
	assert(program == 0);  // run prepare() only once
	Program SkyBox(ShaderFactory::VERT_SKYBOX, ShaderFactory::FRAG_TEXTURE_CUBE);
	assert(SkyBox.getUniformLocation("viewProjection") == Shader::UNIFORM_MAT_VIEW_PROJECTION);
	assert(SkyBox.getUniformLocation("texture0") == Shader::UNIFORM_TEX_TEXTURE0);
	program = SkyBox.release();
	
	glGenVertexArrays(1, &vao);
	glGenBuffers(sizeofArray(vbo), vbo);
}

void SkyBox::upload() const
{
	assert(vao != 0);
	glBindVertexArray(vao);
	
	// vertex position will be used as 3D texture coordinate in skybox.vert shader. The magnitude
	// of the direction vector doesn't matter. As long as a direction is supplied, OpenGL retrieves
	// the corresponding texels that the direction hits (eventually) and returns the properly
	// sampled texture value.
	vec3f size(1, 1, 1);  // size can be uniformly scaled here.
	std::vector<vec3f> positions = Cube::getVertexData(size);
	std::vector<uint8_t> indices = Cube::getVertexIndex(Primitive::TRIANGLES);
	
	// Cube use Z up, while OpenGL use Y up.
	// (x, y, z) rotate X clockwise 90 degrees to suits texture cube's texcoord.
	// (y, z) * (cos(-pi/2), sin(-pi/2) = (y, z) * (0, -1) = (z, -y)
	// texcoord = vec3(position.x, position.z, -position.y);

	const auto& [vbo_position, vbo_index] = vbo;
	GL::bindVertexBuffer(vbo_position, Shader::ATTRIBUTE_VEC_POSITION, positions);
	GL::bindIndexBuffer(vbo_index, indices);
	
	glBindVertexArray(0);
}

mat4f SkyBox::getViewProjection(const mat4f& view, const mat4f& projection)
{
	mat4f view2 = view;
	view2[3][0] = view2[3][1] = view2[3][2] = 0;  // clear translation. important!
	return Camera::multiply(view2, projection);
}

void SkyBox::render(const mat4f& view, const mat4f& projection) const
{
	mat4f viewProjection = SkyBox::getViewProjection(view, projection);
	render(viewProjection);
}

void SkyBox::render(const mat4f& viewProjection) const
{
	if(texture == nullptr)
		return;
	
	assert(program != 0);
	Program::use(program);
	Program::setUniform(Shader::UNIFORM_MAT_VIEW_PROJECTION, viewProjection);
	texture->bind(0);
	
	// Notice the swizzling sentence `gl_Position = position.xyww;` in skybox.vert, SkyBox's depth 
	// will always be 1. write SkyBox color when commingDepth <= historDepth
	glDepthFunc(GL_LEQUAL);
	
	// textureCube use Cube::getIndexData(), and it is counter clockwise when viewed from outside,
	// when we enter the box, we need to reverse winding order to clockwise. To keep a good habit of
	// using GL_CCW the whole life, Culling front face can be used.
	glCullFace(GL_FRONT);  // or glFrontFace(GL_CW);
	
	glBindVertexArray(vao);
	size_t size = Cube::getIndexSize(Primitive::TRIANGLES);
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_BYTE, nullptr);
	glBindVertexArray(0);
	
	glDepthFunc(GL_LESS);  // set depth function back to default
	glCullFace(GL_BACK);  // or glFrontFace(GL_CCW);
}
