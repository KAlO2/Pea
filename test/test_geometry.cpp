#include "test/catch.hpp"

#include <iostream>

#include "geometry/Cylinder.h"
#include "geometry/Grid.h"
#include "geometry/Sphere.h"
#include "geometry/Torus.h"
#include "util/Log.h"
#include "util/utility.h"

using namespace pea;


static const char* tag = "[geometry]";  // used by Catch2
static const char* TAG = "geometry";   // used by log


template <typename T>
inline void matchArray(const std::vector<T>& actual, const T* expect, size_t length)
{
	REQUIRE(actual.size() == length);
	for(size_t i = 0; i < length; ++i)
		REQUIRE(actual[i] == expect[i]);
}

#define MATCH_ARRAY(actualVector, expectArray)  matchArray(actualVector, expectArray, sizeofArray(expectArray))

TEST_CASE("triangleStripToTriangles even", tag)
{
	uint32_t strip[] = {0, 1, 2, 3};
	uint32_t trianglesExpect[] = {0, 1, 2, 2, 1, 3};
	std::vector<uint32_t> triangles = triangleStripToTriangles(strip, sizeofArray(strip));
	MATCH_ARRAY(triangles, trianglesExpect);
}

TEST_CASE("triangleStripToTriangles odd", tag)
{
	uint32_t strip[] = {0, 1, 2, 3, 4};
	uint32_t trianglesExpect[] = {0, 1, 2, 2, 1, 3, 2, 3, 4};
	std::vector<uint32_t> triangles = triangleStripToTriangles(strip, sizeofArray(strip));
	MATCH_ARRAY(triangles, trianglesExpect);
}

TEST_CASE("triangleStripToTriangles degenerated 1", tag)
{
	uint32_t strip[] = {0, 1, 2, 2, 3, 4};
	uint32_t trianglesExpect[] = {0, 1, 2, 3, 2, 4};
	std::vector<uint32_t> triangles = triangleStripToTriangles(strip, sizeofArray(strip));
	MATCH_ARRAY(triangles, trianglesExpect);
}

TEST_CASE("triangleStripToTriangles degenerated 2", tag)
{
	const uint32_t strip[] =
	{
		0, 1, 2, 3,
		3, 4,
		4, 5, 6, 7,
	};
	const uint32_t trianglesExpect[] =
	{
		0, 1, 2, 2, 1, 3,
		4, 5, 6, 6, 5, 7,
	};
	
	std::vector<uint32_t> triangles = triangleStripToTriangles(strip, sizeofArray(strip));
	MATCH_ARRAY(triangles, trianglesExpect);
}

TEST_CASE("quadrilateralsToTriangles", tag)
{
	const uint32_t quadrilaterals[] =
	{
		0, 1, 2, 3,
		2, 3, 4, 5,
		4, 5, 6, 7,
	};
	const uint32_t trianglesExpect[] =
	{
		0, 1, 2, 0, 2, 3,
		2, 3, 4, 2, 4, 5,
		4, 5, 6, 4, 6, 7,
	};
	
	std::vector<uint32_t> triangles = quadrilateralsToTriangles(quadrilaterals, sizeofArray(quadrilaterals));
	MATCH_ARRAY(triangles, trianglesExpect);
}

TEST_CASE("polygonsToTriangles", tag)
{
	const uint32_t polygons[] =
	{
		1, 0, 2, 3, 4,
		3, 4, 5, 6, 7, 8,
		7, 8, 9
	};
	
	const uint32_t sizes[] = {5, 6, 3};
	
	uint32_t trianglesExpect[] =
	{
		1, 0, 2, 1, 2, 3, 1, 3, 4,
		3, 4, 5, 3, 5, 6, 3, 6, 7, 3, 7, 8,
		7, 8, 9
	};
	
	std::vector<uint32_t> triangles = polygonsToTriangles(polygons, sizes, sizeofArray(sizes));
	MATCH_ARRAY(triangles, trianglesExpect);
}

TEST_CASE("Plane", tag)
{

}

TEST_CASE("Sphere", tag)
{
	Sphere s0(vec3f(0, 0, 0), 2), s1(vec3f(1, 0, 0), 1), s2(vec3f(6, 0, 0), 2);

	Sphere s = s0;
	s.merge(s0);
	REQUIRE(s.getPosition() == s0.getPosition());
	REQUIRE(s.getRadius() == s0.getRadius());

	s.merge(s1);
	REQUIRE(s.getPosition() == s0.getPosition());
	REQUIRE(s.getRadius() == s0.getRadius());

	s = s0;
	s.merge(s2);
	REQUIRE(s.getPosition() == vec3f(3, 0, 0));
	REQUIRE(s.getRadius() == 5);
}

TEST_CASE("Sphere slice=4, stack=2 without seam", tag)
{
	// slice = 4, stack = 2 forms a regular octahedron.
	const vec3f verticesExpected[] =
	{
		vec3f(0, 0, +1),  // top
		vec3f(1, 0,  0), vec3f(0, 1, 0), vec3f(-1, 0, 0), vec3f(0, -1, 0),  // XOY plane
		vec3f(0, 0, -1),  // bottom
	};
	
	const uint32_t pointsIndicesExpect[] =
	{
		0, 1, 2, 3, 4, 5
	};
	
	const uint32_t lineStripIndicesExpect[] =
	{
		0,  // stack 0
		1, 2, 3, 4, 1,  // stack 1
		5,  // stack 2
		2, 0, 3, 5, 4, 0,  // slices
	};
	
	const uint32_t triangleStripIndicesExpect[] =
	{
		0, 1, 0, 2, 0, 3, 0, 4, 0, 1,  // top
		1, 5, 2, 5, 3, 5, 4, 5, 1, 5,  // bottom
	};
	
	const uint32_t trianglesIndicesExpect[] =
	{
		0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1,  // top stack
		1, 5, 2, 2, 5, 3, 3, 5, 4, 4, 5, 1,  // bottom stack
	};
	
	const uint32_t triangleFanIndicesExpect[] =
	{
		0, 1, 2, 3, 4, 1,  // top fan
		5, 4, 3, 2, 1, 4,  // bottom fan
	};
	
	Sphere sphere(vec3f(0, 0, 0), 1.0F);
	Sphere::setSlice(4);
	Sphere::setStack(2);
	Sphere::setSeam(0);

	std::vector<vec3f> vertices = sphere.getVertexData();
	MATCH_ARRAY(vertices, verticesExpected);

	std::vector<uint32_t> indices = Sphere::getVertexIndex(Primitive::POINTS);
	MATCH_ARRAY(indices, pointsIndicesExpect);

	indices = Sphere::getVertexIndex(Primitive::LINE_STRIP);
	MATCH_ARRAY(indices, lineStripIndicesExpect);

	indices = Sphere::getVertexIndex(Primitive::TRIANGLE_STRIP);
	MATCH_ARRAY(indices, triangleStripIndicesExpect);

	indices = Sphere::getVertexIndex(Primitive::TRIANGLES);
	MATCH_ARRAY(indices, trianglesIndicesExpect);

	indices = Sphere::getVertexIndex(Primitive::TRIANGLE_FAN);
	MATCH_ARRAY(indices, triangleFanIndicesExpect);

	indices = Sphere::getVertexIndex(Primitive::QUADRILATERALS);
	REQUIRE(indices.empty());
}
#if 0
TEST_CASE("Sphere slice=4, stack=2 with seam", tag)
{
	// slice = 4, stack = 2 forms a regular octahedron.
	const vec3f verticesExpected[] =
	{
		vec3f(0, 0, +1),  // top
		vec3f(1, 0,  0), vec3f(0, 1, 0), vec3f(-1, 0, 0), vec3f(0, -1, 0),  // XOY plane
		vec3f(1, 0,  0), vec3f(0, 1, 0), vec3f(-1, 0, 0), vec3f(0, -1, 0),  // XOY plane
		vec3f(0, 0, -1),  // bottom
	};
	
	const uint32_t pointsIndicesExpect[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9
	};
	
	// 3 line strips
//	const uint32_t lineStripIndicesExpect[] =
	const uint32_t linesIndicesExpect[] =
	{
		0, 1, 0, 2, 0, 3, 0, 4,
		1, 2, 2, 3, 3, 4, 4, 1,
		5, 6, 6, 7, 7, 8, 8, 5,
		5, 9, 6, 9, 7, 9, 8, 9,
	};
	
//	const uint32_t triangleStripIndicesExpect[] =
	const uint32_t trianglesIndicesExpect[] =
	{
		0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1,  // top stack
		5, 9, 6, 6, 9, 7, 7, 9, 8, 8, 9, 5,  // bottom stack
	};
	
	Sphere sphere(vec3f(0, 0, 0), 1.0F);
	Sphere::setSlice(4);
	Sphere::setStack(2);
	Sphere::setSeam(1);
	
	std::vector<vec3f> vertices = sphere.getVertexData();
	MATCH_ARRAY(vertices, verticesExpected);

	std::vector<uint32_t> indices = Sphere::getVertexIndex(Primitive::POINTS);
	MATCH_ARRAY(indices, pointsIndicesExpect);
	
	indices = Sphere::getVertexIndex(Primitive::LINES);
	MATCH_ARRAY(indices, linesIndicesExpect);

	indices = Sphere::getVertexIndex(Primitive::TRIANGLES);
	MATCH_ARRAY(indices, trianglesIndicesExpect);
}
#endif
TEST_CASE("Sphere slice=3, stack=3 without seam", tag)
{
	// slice = 3, stack = 3
	const float c = std::cos(M_PI / 3);
	const float s = std::sin(M_PI / 3);
	vec3f verticesExpected[] =
	{
		vec3f( 0, 0, +1),  // top
		vec3f(+s, 0, +c), vec3f(-c * s, s * s, +c), vec3f(-c * s, -s * s, +c),  // stack 1
		vec3f(+s, 0, -c), vec3f(-c * s, s * s, -c), vec3f(-c * s, -s * s, -c),  // stack 2
		vec3f( 0, 0, -1),  // bottom
	};
	
	const uint32_t pointsIndicesExpect[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7
	};
	
	const uint32_t lineStripIndicesExpect[] =
	{
		0,
		1, 2, 3, 1,
		4, 5, 6, 4,
		7,  // all stacks with first slice
		5, 2, 0, 3, 6, 7
	};
	
	const uint32_t triangleStripIndicesExpect[] =
	{
		0, 1, 0, 2, 0, 3, 0, 1,  // top stack
		1, 4, 2, 5, 3, 6, 1, 4,  // middle stack
		4, 7, 5, 7, 6, 7, 4, 7,  // bottom stack
	};
	
	const uint32_t trianglesIndicesExpect[] =
	{
		0, 1, 2, 0, 2, 3, 0, 3, 1,  // top stack
		1, 4, 2, 2, 4, 5,  // middle stack 1st triangulated face
		2, 5, 3, 3, 5, 6,  // middle stack 2nd triangulated face
		3, 6, 1, 1, 6, 4,  // middle stack 3rd triangulated face
		4, 7, 5, 5, 7, 6, 6, 7, 4,  // bottom stack
	};
	
	const uint32_t triangleFanIndicesExpect[] =
	{
		0, 1, 2, 3, 1,  // top fan
		7, 6, 5, 4, 6,  // bottom fan
	};
	
	const uint32_t quadrilateralsIndicesExpect[] =
	{
		1, 4, 5, 2,
		2, 5, 6, 3,
		3, 6, 4, 1,
	};
	
	Sphere sphere(vec3f(0, 0, 0), 1.0F);
	Sphere::setSlice(3);
	Sphere::setStack(3);
	Sphere::setSeam(0);
	
	constexpr size_t vertexCount = sizeofArray(verticesExpected);
	REQUIRE(Sphere::getVertexSize() == vertexCount);
	std::vector<vec3f> vertices = sphere.getVertexData();
	MATCH_ARRAY(vertices, verticesExpected);

	std::vector<uint32_t> indices = Sphere::getVertexIndex(Primitive::POINTS);
	MATCH_ARRAY(indices, pointsIndicesExpect);
	
	indices = Sphere::getVertexIndex(Primitive::LINE_STRIP);
	MATCH_ARRAY(indices, lineStripIndicesExpect);

	indices = Sphere::getVertexIndex(Primitive::TRIANGLE_STRIP);
	MATCH_ARRAY(indices, triangleStripIndicesExpect);
	
	indices = Sphere::getVertexIndex(Primitive::TRIANGLES);
	MATCH_ARRAY(indices, trianglesIndicesExpect);
	
	indices = Sphere::getVertexIndex(Primitive::TRIANGLE_FAN);
	MATCH_ARRAY(indices, triangleFanIndicesExpect);
	
	indices = Sphere::getVertexIndex(Primitive::QUADRILATERALS);
	MATCH_ARRAY(indices, quadrilateralsIndicesExpect);
}

TEST_CASE("pitch yaw forward", tag)
{
	// pitch = 0, yaw = -pi, -pi/2, 0, pi/2.
	//     X0Y plane            XOZ plane
	//        ^ y                   
	//        | +pi/2              | -pi/2
	//        |                    |
	//        |     0        -pi   |
	//  ------+------> x     ------+---------> x
	// -pi    |                    |       0
	//        |                    |
	//        | -pi/2              | +pi/2
	//                             V z
	constexpr uint8_t N = 4;
	const float yaws[N] = { -M_PI, -M_PI / 2, 0, M_PI / 2};
	const vec3f forwards[N] =
	{
#if 1 //USE_MATH_XYZ
		vec3f(-1, 0, 0), vec3f(0, -1, 0), vec3f(1, 0, 0), vec3f(0, 1, 0)
#else
		vec3f(-1, 0, 0), vec3f(0, 0, -1), vec3f(1, 0, 0), vec3f(0, 0, 1)
#endif
	};
	
	for(std::remove_const<decltype(N)>::type i = 0; i < N; ++i)
	{
		const float pitch = 0.0F, &yaw = yaws[i];
		const vec3f& forward = forwards[i];
		REQUIRE(Sphere::composeOrientation(pitch, yaw) == forward);
		
		const auto& pair = Sphere::decomposeOrientation(forward);
		REQUIRE(pair.first == pitch);
		REQUIRE(pair.second == yaw);
	}
	
	// cube vertex (1, 1, 1) in the first Octant
	vec3f vertex(1, 1, 1);
	float pitch = std::atan2(vertex.z, std::hypot(vertex.x, vertex.y));
	float yaw = M_PI / 4;
	
	vec3f forward = vertex.normalize();
	REQUIRE(Sphere::composeOrientation(pitch, yaw) == forward);
	REQUIRE(Sphere::decomposeOrientation(forward) == std::make_pair(pitch, yaw));
}

TEST_CASE("Grid", tag)
{
	uint32_t stepsX = 3, stepsY = 2;
	float step = 1.0F;
	
	const vec3f positionsExpected[] =
	{
		vec3f(-1.5F, -1.0F, 0), vec3f(-0.5F, -1.0F, 0), vec3f(+0.5F, -1.0F, 0), vec3f(+1.5F, -1.0F, 0),
		vec3f(-1.5F, +0.0F, 0), vec3f(-0.5F, +0.0F, 0), vec3f(+0.5F, +0.0F, 0), vec3f(+1.5F, -0.0F, 0),
		vec3f(-1.5F, +1.0F, 0), vec3f(-0.5F, +1.0F, 0), vec3f(+0.5F, +1.0F, 0), vec3f(+1.5F, +1.0F, 0),
	};
	
	std::vector<vec3f> positions = Grid::getVertexData(stepsX, stepsY, step);
	MATCH_ARRAY(positions, positionsExpected);
	
	const vec2f texcoordsExpected[] =
	{
		vec2f(0.0F, 0.0F), vec2f(1.0F/3, 0.0F), vec2f(2.0F/3, 0.0F), vec2f(1.0F, 0.0F),
		vec2f(0.0F, 0.5F), vec2f(1.0F/3, 0.5F), vec2f(2.0F/3, 0.5F), vec2f(1.0F, 0.5F),
		vec2f(0.0F, 1.0F), vec2f(1.0F/3, 1.0F), vec2f(2.0F/3, 1.0F), vec2f(1.0F, 1.0F),
	};
	
	std::vector<vec2f> texcoords = Grid::getTexcoordData(1.0F, 1.0F, stepsX, stepsY);
	MATCH_ARRAY(texcoords, texcoordsExpected);
	
/*
	8---9---10--11
	|   |   |   |
	4---5---6---7
	|   |   |   |
	0---1---2---3
*/
	const uint32_t pointIndicesExpect[] =
	{
		0, 1, 2, 3,
		4, 5, 6, 7,
		8, 9, 10, 11,
	};
	
	const uint32_t lineIndicesExpect[] =
	{
		0, 1, 1, 2, 2, 3,
		0, 4, 1, 5, 2, 6, 3, 7,
		4, 5, 5, 6, 6, 7,
		4, 8, 5, 9, 6, 10, 7, 11,
		8, 9, 9, 10, 10, 11
	};
	
	const uint32_t triangleIndicesExpect[] =
	{
		4, 0, 5, 5, 0, 1,  5, 1, 6, 6, 1, 2,  6, 2, 7, 7, 2, 3,
		8, 4, 9, 9, 4, 5,  9, 5, 10, 10, 5, 6, 10, 6, 11, 11, 6, 7,
	};
	
	const uint32_t triangleStripIndicesExpect[] =
	{
		4, 0, 5, 1, 6, 2, 7, 3,  3, 8,
		8, 4, 9, 5,10, 6,11, 7,
	};
	
	const uint32_t quadrilateralIndicesExpect[] =
	{
		4, 0, 1, 5,  5, 1, 2, 6,  6, 2, 3, 7,
		8, 4, 5, 9,  9, 5, 6, 10, 10, 6, 7, 11,
	};
	
	std::vector<uint32_t> indices = Grid::getIndexData(stepsX, stepsY, Primitive::POINTS);
	MATCH_ARRAY(indices, pointIndicesExpect);
	
	indices = Grid::getIndexData(stepsX, stepsY, Primitive::LINES);
	MATCH_ARRAY(indices, lineIndicesExpect);
	
	indices = Grid::getIndexData(stepsX, stepsY, Primitive::TRIANGLES);
	MATCH_ARRAY(indices, triangleIndicesExpect);
	
	indices = Grid::getIndexData(stepsX, stepsY, Primitive::TRIANGLE_STRIP);
	MATCH_ARRAY(indices, triangleStripIndicesExpect);
	
	indices = Grid::getIndexData(stepsX, stepsY, Primitive::QUADRILATERAL_STRIP);
	MATCH_ARRAY(indices, triangleStripIndicesExpect);  // same result
	
	indices = Grid::getIndexData(stepsX, stepsY, Primitive::QUADRILATERALS);
	MATCH_ARRAY(indices, quadrilateralIndicesExpect);
}

TEST_CASE("Cylinder without cap", tag)
{
	constexpr float radius = 1, height = 2;
	Cylinder cylinder(radius, height);
	Cylinder::setSlice(4);
	Cylinder::setCapFillType(CapFillType::NONE);
	
	const vec3f positionsExpected[] =
	{
		vec3f(+1,  0, +1), vec3f(+1,  0, -1),
		vec3f( 0,  1, +1), vec3f( 0,  1, -1),
		vec3f(-1,  0, +1), vec3f(-1,  0, -1),
		vec3f( 0, -1, +1), vec3f( 0, -1, -1),
	};
	std::vector<vec3f> positions = cylinder.getVertexData();
	MATCH_ARRAY(positions, positionsExpected);
	
	const uint32_t triangleStripIndicesExpected[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 0, 1,
	};
	std::vector<uint32_t> indices = Cylinder::getVertexIndex(Primitive::TRIANGLE_STRIP);
	MATCH_ARRAY(indices, triangleStripIndicesExpected);
	
	const vec3f normalsExpected[] =
	{
		vec3f(+1, 0, 0),
		vec3f(0, +1, 0),
		vec3f(-1, 0, 0),
		vec3f(0, -1, 0),
	};
	std::vector<vec3f> normals = Cylinder::getNormalData();
	MATCH_ARRAY(normals, normalsExpected);
	
	
}

TEST_CASE("Cylinder with triangle fan", tag)
{
	constexpr float radius = 1, height = 2;
	Cylinder cylinder(radius, height);
	Cylinder::setSlice(4);
	Cylinder::setCapFillType(CapFillType::TRIANGLE_FAN);
	
	const vec3f positionsExpected[] =
	{
		vec3f(+1,  0, +1), vec3f(+1,  0, -1),
		vec3f( 0,  1, +1), vec3f( 0,  1, -1),
		vec3f(-1,  0, +1), vec3f(-1,  0, -1),
		vec3f( 0, -1, +1), vec3f( 0, -1, -1),
		vec3f( 0,  0, +1), vec3f( 0,  0, -1),
	};
	
	const uint32_t trianglesIndicesExpected[] =
	{
		8, 0, 2, 8, 2, 4, 8, 4, 6, 8, 6, 0,  // top
		0, 1, 2, 2, 1, 3,
		2, 3, 4, 4, 3, 5,
		4, 5, 6, 6, 5, 7,
		6, 7, 0, 0, 7, 1,
		9, 1, 7, 9, 7, 5, 9, 5, 3, 9, 3, 1,  // bottom
	};
	
	const uint32_t triangleStripIndicesExpected[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 0, 1
	};
	
	const uint32_t triangleFanIndicesExpected[] =
	{
		8, 0, 2, 4, 6, 0,  // top fan
		9, 1, 7, 5, 3, 1,  // bottom fan
	};
	
	const uint32_t lineStripIndicesExpected[] =
	{
		8,  // start from top
		0, 2, 4, 6, 0,
		1, 3, 5, 7, 1,
		9,
		3, 2, 8, 4, 5, 9, 7, 6, 8
	};
	
	std::vector<vec3f> positions = cylinder.getVertexData();
	MATCH_ARRAY(positions, positionsExpected);
	
	std::vector<uint32_t> indices = Cylinder::getVertexIndex(Primitive::TRIANGLES);
	MATCH_ARRAY(indices, trianglesIndicesExpected);
	
	indices = Cylinder::getVertexIndex(Primitive::TRIANGLE_STRIP);
	MATCH_ARRAY(indices, triangleStripIndicesExpected);
	
	indices = Cylinder::getVertexIndex(Primitive::TRIANGLE_FAN);
	MATCH_ARRAY(indices, triangleFanIndicesExpected);
	
	indices = Cylinder::getVertexIndex(Primitive::LINE_STRIP);
	MATCH_ARRAY(indices, lineStripIndicesExpected);
}

TEST_CASE("Cylinder with polygon", tag)
{
	constexpr float radius = 1, height = 2;
	Cylinder cylinder(radius, height);
	Cylinder::setSlice(4);
	Cylinder::setCapFillType(CapFillType::POLYGON);
	
	const vec3f positionsExpected[] =
	{
		vec3f(+1,  0, +1), vec3f(+1,  0, -1),
		vec3f( 0,  1, +1), vec3f( 0,  1, -1),
		vec3f(-1,  0, +1), vec3f(-1,  0, -1),
		vec3f( 0, -1, +1), vec3f( 0, -1, -1),
	};
	
	std::vector<vec3f> positions = cylinder.getVertexData();
	MATCH_ARRAY(positions, positionsExpected);
	
	const uint32_t triangleStripIndicesExpected[] =
	{
		0, 1, 2, 3, 4, 5, 6, 7, 0, 1
	};
	
	const uint32_t polygonsIndicesExpected[] =
	{
		0, 2, 4, 6,
		1, 7, 5, 3,  // 1, 3, 5, 7 CCW
	};
	
	std::vector<uint32_t> indices = Cylinder::getVertexIndex(Primitive::TRIANGLE_STRIP);
	MATCH_ARRAY(indices, triangleStripIndicesExpected);
	
	indices = Cylinder::getVertexIndex(Primitive::POLYGON);
	MATCH_ARRAY(indices, polygonsIndicesExpected);
}

TEST_CASE("Torus without seam", tag)
{
	float ringRadius = 1.0, tubeRadius = 1.0;
	Torus torus(ringRadius, tubeRadius);
	torus.setSlice(4);
	torus.setTubeSubdivision(4);
	torus.markSeam(false);
	
	const vec3f positionsExpected[] =
	{
		vec3f( 0, 0,  0), vec3f(0,  0,  0), vec3f( 0, 0,  0), vec3f(0,  0,  0),
		vec3f(+1, 0, -1), vec3f(0, +1, -1), vec3f(-1, 0, -1), vec3f(0, -1, -1),
		vec3f(+2, 0,  0), vec3f(0, +2,  0), vec3f(-2, 0,  0), vec3f(0, -2,  0),
		vec3f(+1, 0, +1), vec3f(0, +1, +1), vec3f(-1, 0, +1), vec3f(0, -1, +1),
	};
	
	std::vector<vec3f> positions = torus.getVertexData();
	MATCH_ARRAY(positions, positionsExpected);
	
	const uint32_t indicesExpected[] =
	{
		4, 0, 5, 1, 6, 2, 7, 3, 4, 0,
		0, 8,  // degenerated
		8, 4, 9, 5, 10, 6, 11, 7, 8, 4,
		4, 12,  // degenerated
		12, 8, 13, 9, 14, 10, 15, 11, 12 , 8,
		8, 0,  // degenerated
		0, 12, 1, 13, 2, 14, 3, 15, 0, 12,
	};
	
	std::vector<uint32_t> indices = torus.getVertexIndex(Primitive::TRIANGLE_STRIP);
	MATCH_ARRAY(indices, indicesExpected);
	
//	std::vector<vec2f> texcoords = torus.getTexcoordData();
	const vec3f normalsExpected[] =
	{
		vec3f(-1, 0,  0), vec3f(0, -1,  0), vec3f(+1, 0,  0), vec3f(0, +1,  0),
		vec3f( 0, 0, -1), vec3f(0,  0, -1), vec3f( 0, 0, -1), vec3f(0,  0, -1),
		vec3f(+1, 0,  0), vec3f(0, +1,  0), vec3f(-1, 0,  0), vec3f(0, -1,  0),
		vec3f( 0, 0, +1), vec3f(0,  0, +1), vec3f( 0, 0, +1), vec3f(0,  0, +1),
	};
	
	std::vector<vec3f> normals = torus.getNormalData();
	MATCH_ARRAY(normals, normalsExpected);
	
	std::vector<uint32_t> triangleIndicesExpected = triangleStripToTriangles(indicesExpected, sizeofArray(indicesExpected));
	indices = torus.getVertexIndex(Primitive::TRIANGLES);
	matchArray(indices, triangleIndicesExpected.data(), triangleIndicesExpected.size());
}

TEST_CASE("Torus with seam", tag)
{
	Torus torus(1.0, 1.0);
	torus.setSlice(4);
	torus.setTubeSubdivision(4);
	torus.markSeam(true);
	
	const vec3f positionsExpected[] =
	{
		vec3f( 0, 0,  0), vec3f(0,  0,  0), vec3f( 0, 0,  0), vec3f(0,  0,  0),
		vec3f(+1, 0, -1), vec3f(0, +1, -1), vec3f(-1, 0, -1), vec3f(0, -1, -1),
		vec3f(+2, 0,  0), vec3f(0, +2,  0), vec3f(-2, 0,  0), vec3f(0, -2,  0),
		vec3f(+1, 0, +1), vec3f(0, +1, +1), vec3f(-1, 0, +1), vec3f(0, -1, +1),
	};
	// last row and column are the same as first.
	std::vector<vec3f> positions = torus.getVertexData();
	MATCH_ARRAY(positions, positionsExpected);
	
	const uint32_t indicesExpected[] =
	{
		4, 0, 5, 1, 6, 2, 7, 3, 4, 0,
		0, 8,  // degenerated
		8, 4, 9, 5, 10, 6, 11, 7, 8, 4,
		4, 12,  // degenerated
		12, 8, 13, 9, 14, 10, 15, 11, 12, 8,
		8, 0,  // degenerated
		0, 12, 1, 13, 2, 14, 3, 15, 0, 12,
	};
	
	std::vector<uint32_t> indices = torus.getVertexIndex(Primitive::TRIANGLE_STRIP);
	MATCH_ARRAY(indices, indicesExpected);
	
	const vec3f normalsExpected[] =
	{
		vec3f(-1, 0,  0), vec3f(0, -1,  0), vec3f(+1, 0,  0), vec3f(0, +1,  0),
		vec3f( 0, 0, -1), vec3f(0,  0, -1), vec3f( 0, 0, -1), vec3f(0,  0, -1),
		vec3f(+1, 0,  0), vec3f(0, +1,  0), vec3f(-1, 0,  0), vec3f(0, -1,  0),
		vec3f( 0, 0, +1), vec3f(0,  0, +1), vec3f( 0, 0, +1), vec3f(0,  0, +1),
	};
	
	std::vector<vec3f> normals = torus.getNormalData();
	MATCH_ARRAY(normals, normalsExpected);
}
