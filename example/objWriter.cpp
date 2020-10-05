#include "geometry/Cube.h"
#include "geometry/Sphere.h"
#include "geometry/Tetrahedron.h"
#include "io/Group.h"
#include "io/Model.h"
#include "io/Model_OBJ.h"
#include "util/Log.h"

using namespace pea;

void writeTriangulatedCube(const std::string& name, float edgeLength)
{
	vec3f cubeSize(edgeLength, edgeLength, edgeLength);
	std::vector<vec3f> vertices = Cube::getVertexData(cubeSize);
	{
		std::vector<uint8_t> indices = Cube::getVertexIndex(Primitive::TRIANGLES);
		vertices = Group::dropIndex(vertices, indices);
	}
	
	Model model;
	model.addVertex(vertices);
	
	const size_t vertexSize = vertices.size();
	std::vector<uint32_t> indices(vertexSize);
//	std::iota(indices.begin(), indices.end(), 0);
	for(size_t i = 0; i < vertexSize; ++i)
		indices[i] = i;
	
	constexpr uint8_t N = 3;
	for(size_t i = 0; i < vertexSize; i += N)
		model.addFace(indices.data() + i, N);
	
	Model_OBJ object(model);
	object.save(".", name);
}

void writeCube(const std::string& name, float edgeLength)
{
	vec3f cubeSize(edgeLength, edgeLength, edgeLength);
	std::vector<vec3f> vertices = Cube::getVertexData(cubeSize);
	std::vector<uint8_t> indices = Cube::getVertexIndex(Primitive::QUADRILATERALS);
	
	Model model;
	model.addVertex(vertices);
	
	const size_t indexSize = indices.size();
	constexpr uint8_t N = 4;
	for(size_t i = 0; i < indexSize; i += N)
	{
		uint32_t quadrilateral[N];
		for(uint8_t j = 0; j < N; ++j)
			quadrilateral[j] = indices[i + j];
		model.addFace(quadrilateral, N);
	}
	
	std::string names[Cube::FACE_COUNT] = {"left", "right", "back", "front", "bottom", "top"};
	for(uint8_t i = 0; i < Cube::FACE_COUNT; ++i)
	{
		Group group(GroupType::FACE);
		group.indices.push_back(i);
		model.addGroup(names[i], group);
	}

	// and once for all
	Group group(GroupType::FACE);
	for(uint32_t i = 0; i < Cube::FACE_COUNT; ++i)
		group.indices.push_back(i);
	model.addGroup("cube", group);

	Model_OBJ object(model);
	object.save(".", name);
}

void writeTetrahedron(const std::string& name, float edgeLength)
{
	std::vector<vec3f> vertices = Tetrahedron::getVertexData(edgeLength);
	std::vector<uint8_t> indices = Tetrahedron::getVertexIndex(Primitive::TRIANGLES);
	
	Model model;
	model.addVertex(vertices);
	
	const size_t indexSize = indices.size();
	constexpr uint8_t N = 3;
	for(size_t i = 0; i < indexSize; i += N)
	{
		uint32_t triangle[N];
		for(uint8_t j = 0; j < N; ++j)
			triangle[j] = indices[i + j];
		model.addFace(triangle, N);
	}
	
	Model_OBJ object(model);
	object.save(".", name);
}

void writeSphere(const std::string& name, const Sphere& sphere, bool triangulated = false)
{
	Sphere::setSlice(64);
	Sphere::setStack(32);
	std::vector<vec3f> vertices = sphere.getVertexData();
	std::vector<uint32_t> fan2Indices = Sphere::getVertexIndex(Primitive::TRIANGLE_FAN);
	std::vector<uint32_t> quadIndices = Sphere::getVertexIndex(Primitive::QUADRILATERALS);
	
	Model model;
	model.addVertex(vertices);
	
	// top fan
	uint32_t fanVertexSize = fan2Indices.size() / 2;
	std::vector<uint32_t> indices = triangleFanToTriangles(fan2Indices.data(), fanVertexSize);
	const uint32_t* data = indices.data();
	for(size_t i = 0; i < indices.size(); i += 3, data += 3)
		model.addFace(data, 3);

	if(triangulated)
	{
		indices = quadrilateralsToTriangles(quadIndices.data(), quadIndices.size());
		data = indices.data();
		for(size_t i = 0; i < indices.size(); i += 3, data += 3)
			model.addFace(data, 3);
	}
	else
	{
		data = quadIndices.data();
		for(size_t i = 0; i < quadIndices.size(); i += 4, data += 4)
			model.addFace(data, 4);
	}
	
	// bottom fan
	indices = triangleFanToTriangles(fan2Indices.data() + fanVertexSize, fanVertexSize);
	data = indices.data();
	for(size_t i = 0; i < indices.size(); i += 3, data += 3)
		model.addFace(data, 3);
	
	Model_OBJ object(model);
	object.save(".", name);
}

int main()
{
//	slog.setLevel(Log::LEVEL_INFO);
	
	writeTriangulatedCube("cube.triangulated", 2.0);
	writeCube("cube", 2.0);
	writeTetrahedron("tetrahedron", 1.0);
	
	vec3f center(1, 1, 1);
	float radius = 1;
	Sphere sphere(center, radius);
	writeSphere("sphere", sphere);

	const char* README = R""(
Four objects (cube.obj  cube.triangulated.obj  sphere.obj  tetrahedron.obj) have been generated in 
this directory. You can check its content with a text editor, or import it to Blender 3D software.
)"";
	printf("\x1b[0m");
	printf("%s\n", README);
	
	return 0;
}
