#include "geometry/Grid.h"
#include "opengl/Program.h"
#include "opengl/ShaderFactory.h"
#include "opengl/Texture.h"
#include "opengl/GL.h"
#include "io/FileSystem.h"
#include "scene/Mesh.h"
#include "scene/Terrain.h"
#include "scene/World.h"

using namespace pea;

/*
Feature:
	1. displacement mapping;
	2. VTF (Vertex Texture Fetch) https://www.khronos.org/opengl/wiki/Vertex_Texture_Fetch

Description:
	1. Key ASDW or direction keys for move left / back / right / top;
	2. Space bar for jump;
	3. Key C for crouch;
	4. Moving mouse for change viewing horizon.
*/
int main()
{
	int32_t windowWidth = 1280;
	int32_t windowHeight = 720;
	const char* windowTitle = "Displacement Mapping";
	World world(windowWidth, windowHeight, windowTitle);
	world.prepare();
	
	std::string dir = FileSystem::getRelativePath("res/image/skybox/");
	const std::string filenames[6] = {"left.jpg", "right.jpg", "back.jpg", "front.jpg", "bottom.jpg", "top.jpg"};
	Texture textureSky(GL_TEXTURE_CUBE_MAP);
	textureSky.loadCube(dir, filenames);
	textureSky.setParameter(Texture::Parameter(1));
	world.setTextureSky(&textureSky);

	Program program(ShaderFactory::VERT_M_VP_HEIGHT_TEXCOORD, ShaderFactory::FRAG_TEXTURE_RGB);
	
	// https://en.wikipedia.org/wiki/Heightmap
	Texture textureHeight(GL_TEXTURE_2D);
	std::string path = FileSystem::getRelativePath("res/image/height_map2.png");
	Texture::Parameter parameter(1);
	parameter.setMapMode(GL_MIRRORED_REPEAT);
	textureHeight.load(path);
	textureHeight.setParameter(parameter);
	textureHeight.setType(Texture::Type::HEIGHT);
	
	path = FileSystem::getRelativePath("res/image/forest-evergreen.jpg");
	parameter.levels = 10;
	Texture texture(GL_TEXTURE_2D);
	texture.load(path);
	texture.setParameter(parameter);
	
	float left = -50, right = +50;  // meter
	uint32_t stepCount = 800;
	std::vector<vec3f> vertices = Grid::getVertexData(left, right, stepCount);
	std::vector<vec2f> texcoord = Grid::getTexcoordData(1.0F, 1.0F, stepCount, stepCount);
	constexpr Primitive primitive = Primitive::TRIANGLE_STRIP;
	std::vector<uint32_t> indices = Grid::getIndexData(stepCount, stepCount, primitive);
	
	std::unique_ptr<Mesh> mesh = Mesh::Builder(vertices)
			.setTexcoord(texcoord)
			.setIndex(indices)
			.build();
	mesh->setName("terrain");
	mesh->prepare(primitive);
	mesh->upload();
	mesh->setProgram(program.getName());
	mesh->setTexture(textureHeight, 0);
	mesh->setTexture(texture, 1);
	vec3f scale(10, 10, 3);
	mesh->setUniform(Shader::UNIFORM_VEC_SCALE, Type::VEC3F, &scale);
	
	world.addObject(mesh.release());
	
	world.run();
	
	return 0;
}
