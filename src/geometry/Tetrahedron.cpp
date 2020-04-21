#include "geometry/Tetrahedron.h"

using namespace pea;


float Tetrahedron::getInradius(float edgeLength)
{
	const float k = std::sqrt(6.0) / 12;
	return k * edgeLength;
}

float Tetrahedron::getCircumradius(float edgeLength)
{
	// two counter face of a cube
	// diagonal line forms a regular tetrahedron
	// 2 * radius = sqrt(3.0) * k;  edge = sqrt(2.0) * k;
	// radius / edge = sqrt(6) / 4
	const float k = std::sqrt(6.0) / 4;
	return k * edgeLength;
}

std::vector<vec3f> Tetrahedron::getVertexData(float edgeLength)
{
	std::vector<vec3f> vertices(4);
	const float z0 = std::sqrt(6.0) / 4;
	vertices[0] = vec3f(0, 0, z0 * edgeLength);
	const float z1 = edgeLength / -3.0;
	const float r = 1 / std::sqrt(3.0) * edgeLength;
	const float x = r / 2, y = x * sqrt(3.0);
	vertices[1] = vec3f( r,  0, z1);
	vertices[2] = vec3f(-x, +y, z1);
	vertices[3] = vec3f(-x, -y, z1);
	
	return vertices;
}

std::vector<uint8_t> Tetrahedron::getVertexIndex(Primitive primitive)
{
	switch(primitive)
	{
	case Primitive::POINTS:
		return std::vector<uint8_t>({0, 1, 2, 3});
	
	case Primitive::LINES:
		return std::vector<uint8_t>
		({
			0, 1, 0, 2, 0, 3,
			1, 2, 2, 3, 3, 1
		});
	
	case Primitive::TRIANGLES:
		return std::vector<uint8_t>({
			0, 1, 2,
			0, 2, 3,
			0, 3, 1,
			1, 3, 2  // bottom triangle
		});
	
	default:
		return std::vector<uint8_t>{};
	}
}

std::vector<vec3f> Tetrahedron::getNormalData()
{
	std::vector<vec3f> v = getVertexData(1.0F);
	vec3f faceNormals[4] =
	{
		(v[0] + v[1] + v[2]).normalize(),
		(v[0] + v[2] + v[3]).normalize(),
		(v[0] + v[3] + v[1]).normalize(),
		vec3f(0, 0, -1),
	};
	
	constexpr size_t count = 4 * 3; 
	std::vector<vec3f> normals(count);
	for(size_t i = 0; i < count; ++i)
		normals[i] = faceNormals[i / 3];
	return normals;
}

/*
	^
	|t     0
	|     / \
	|    /   \
	|   3_____2
	|  /\    / \
	| /  \  /   \
	|/    \/     \
	0------1-----0---->s
*/
std::vector<vec2f> Tetrahedron::getTexcoordData()
{
	const float s = std::sqrt(3.0);
	vec2f _1(0.50, 0), _2(0.75, s / 4), _3(0.25, s / 4);
	return std::vector<vec2f>({
		vec2f(1, 0),       _1, _2,
		vec2f(0.5, s / 2), _2, _3,
		vec2f(0, 0),       _3, _1,
		_1, _3, _2
	});
}
