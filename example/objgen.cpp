#include <cstring>

#include "geometry/Cube.h"
#include "geometry/Sphere.h"
#include "geometry/Tetrahedron.h"
#include "io/Group.h"
#include "io/Model.h"
#include "io/Model_OBJ.h"
#include "util/Log.h"

#include <getopt.h>

using namespace pea;

void writeTriangulatedCube(const std::string& name, float edgeLength, const vec3f& center = vec3f(0, 0, 0))
{
	vec3f cubeSize(edgeLength, edgeLength, edgeLength);
	std::vector<vec3f> vertices = Cube::getVertexData(cubeSize);
	for(vec3f& vertex: vertices)
		vertex += center;
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

void writeCube(const std::string& name, float edgeLength, const vec3f& center = vec3f(0, 0, 0))
{
	vec3f cubeSize(edgeLength, edgeLength, edgeLength);
	std::vector<vec3f> vertices = Cube::getVertexData(cubeSize);
	for(vec3f& vertex: vertices)
		vertex += center;
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

void writeTetrahedron(const std::string& name, float edgeLength, const vec3f& center = vec3f(0, 0, 0))
{
	std::vector<vec3f> vertices = Tetrahedron::getVertexData(edgeLength);
	for(vec3f& vertex: vertices)
		vertex += center;
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

void writeTeapot()
{
	
}

void usage()
{
	const char* manual = R"(Usage: objgen <option> <file>
 generate Wavefront .obj information to object <file>. Supported geometries are cube, sphere, tetrahedron.
  -h, --help        Display this information
  -g, --geometry    Geometry options {"cube", "sphere", "tetrahedron", "all"}
  -l, --length      Edge length, default to 1.0
  -o, --origin      Object's position, default to (0, 0, 0)
  -r, --radius      Circle or sphere's radius, default to 1.0
  -t, --triangulate Triangulate each face, no quadrilaterals, no polygons.
)";
	printf("%s", manual);
}

int main(int argc, char* argv[])
{
	slog.setLevel(Log::LEVEL_WARNING);
	const struct option longOptions[] =
	{
		{"help",              no_argument, nullptr, 'h'},
		{"geometry",    required_argument, nullptr, 'g'},
		{"length",      required_argument, nullptr, 'l'},
		{"origin",      required_argument, nullptr, 'o'},
		{"radius",      required_argument, nullptr, 'r'},
		{"triangulate",       no_argument, nullptr, 't'},
		{nullptr,                       0, nullptr,   0},
	};
	
	vec3f center(0, 0, 0);
	float edgeLength = 1.0F, radius = 1.0F;
	enum GeometryType {NONE, CUBE, SPHERE, TETRAHEDRON, ALL} type = NONE;
	bool triangulate = false;
	std::string name;
	
	int ch;
	int optionIndex = 0;
	const char* valuePositive = "%s should be a positive value.";
	while((ch = getopt_long(argc, argv, "hg:l:o:r:t", longOptions, &optionIndex)) != -1)
	{
		switch(ch)
		{
		case 'g':
			if(std::strcmp(optarg, "cube") == 0)
			{
				type = GeometryType::CUBE;
				name = "cube";
			}
			else if(std::strcmp(optarg, "sphere") == 0)
			{
				type = GeometryType::SPHERE;
				name = "sphere";
			}
			else if(std::strcmp(optarg, "tetrahedron") == 0)
			{
				type = GeometryType::TETRAHEDRON;
				name = "tetrahedron";
			}
			else if(std::strcmp(optarg, "all") == 0)
				type = GeometryType::ALL;
			else
			{
				printf("unknown geometry \'%s\', option can be {cube, sphere, tetrahedron, all}\n", optarg);
				return -1;
			}
			break;
		
		case 'l':
			edgeLength = std::atoi(optarg);
			if(edgeLength <= 0)
			{
				printf(valuePositive, "length");
				return -2;
			}
			break;
		
		case 'o':
			if(sscanf(optarg, "%f,%f,%f", &center.x, &center.y, &center.z) != 3)
			{
				center = vec3f(0, 0, 0);
				printf("invalid vec3 format %s, should be 1,2.,3.0\n", optarg);
				return -3;
			}
			break;
		
		case 'r':
			radius = std::atoi(optarg);
			if(radius <= 0)
			{
				printf(valuePositive, "radius");
				return -2;
			}
			break;
		
		case 't':
			triangulate = true;
			break;
		
		case 'h':
		default:
			usage();
			return 0;
		}
	}
	
	// object's name is supplied
	if(optind < argc)
		name = argv[optind++];
	
	switch(type)
	{
	case GeometryType::NONE:
		usage();
		return 0;
	
	case GeometryType::CUBE:
		if(triangulate)
			writeTriangulatedCube(name, edgeLength, center);
		else
			writeCube(name, edgeLength, center);
		break;
	
	case GeometryType::SPHERE:
		writeSphere(name, Sphere(center, radius), triangulate);
		break;
	
	case GeometryType::TETRAHEDRON:
		writeTetrahedron(name, edgeLength, center);
		break;
	
	case GeometryType::ALL:
		if(triangulate)
			writeTriangulatedCube("cube", edgeLength, center);
		else
			writeCube("cube", edgeLength);
		writeSphere("sphere", Sphere(center, radius), triangulate);
		writeTetrahedron("tetrahedron", edgeLength);
		break;
	}
	
	return 0;
}
