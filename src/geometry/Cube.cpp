#include "geometry/Cube.h"

#include <algorithm>

#include "util/utility.h"

using namespace pea;

static constexpr uint8_t faceCount = 6;

std::vector<vec3f> Cube::getVertexData(const vec3f& size)
{
	float x = size.x / 2, y = size.y / 2, z = size.z / 2;
	std::vector<vec3f> positions(8);
	
	positions[0] = vec3f(+x, +y, +z);
	positions[1] = vec3f(-x, +y, +z);
	positions[2] = vec3f(-x, -y, +z);
	positions[3] = vec3f(+x, -y, +z);
	
	positions[4] = vec3f(+x, +y, -z);
	positions[5] = vec3f(-x, +y, -z);
	positions[6] = vec3f(-x, -y, -z);
	positions[7] = vec3f(+x, -y, -z);

	return positions;
}

std::vector<vec3f> Cube::getVertexData(const vec3f& min, const vec3f& max)
{
	float x0 = std::min(min.x, max.x), x1 = std::max(min.x, max.x);
	float y0 = std::min(min.y, max.y), y1 = std::max(min.y, max.y);
	float z0 = std::min(min.z, max.z), z1 = std::max(min.z, max.z);
	
	std::vector<vec3f> positions(8);
	
	positions[0] = vec3f(x1, y1, z1);
	positions[1] = vec3f(x0, y1, z1);
	positions[2] = vec3f(x0, y0, z1);
	positions[3] = vec3f(x1, y0, z1);
	
	positions[4] = vec3f(x1, y1, z0);
	positions[5] = vec3f(x0, y1, z0);
	positions[6] = vec3f(x0, y0, z0);
	positions[7] = vec3f(x1, y0, z0);

	return positions;
}

/*
	Vertices are ordered by their octant
	
	    1---------0    
	   /:        /|    
	  / :       / |    z             t
	 /  :      /  |    ^             ^
	 2--:-----3   |    |    y        |    r
	 |  5- - -|- -4    |  7          |  7
	 | /      |  /     | /           | /
	 |/       | /      |/            |/
	 6--------7/       +--------> x  +--------> s
	
 	vec3f(l, l, 0),  //     1--------0     ^ Y
	vec3f(0, l, 0),  //    /:       /|     |
	vec3f(0, l, l),  //   / :      / |     |
	vec3f(l, l, l),  //  2--------3  |     |              0--------> U
	vec3f(l, 0, 0),  //  |  5-----|--/4    0--------> X   |
	vec3f(0, 0, 0),  //  | .      | /     /               |
	vec3f(0, 0, l),  //  |/       |/     /                |
	vec3f(l, 0, l),  //  6--------7     L Z               L V
*/

size_t Cube::getIndexSize(Primitive primitive)
{
	switch(primitive)
	{
	case Primitive::POINTS:          return 8;
	case Primitive::LINES:           return 12 * 2;  // 12 edges
	case Primitive::TRIANGLES:       return 6 * 3 * 2;  // 6 faces
	case Primitive::TRIANGLE_STRIP:  return 8 + 1 + 8;  // 2 strips + 1 primitive restart index
	case Primitive::TRIANGLE_FAN:    return 8 * 2;  // 2 fans
	case Primitive::QUADRILATERALS:  return 6 * 4;
	default:                         return 0;  // unsupported
	}
}

std::vector<uint8_t> Cube::getVertexIndex(Primitive primitive)
{
	switch(primitive)
	{
	case Primitive::POINTS:
		return std::vector<uint8_t>({0, 1, 2, 3, 4, 5, 6, 7});
		
	case Primitive::LINES:
		return std::vector<uint8_t>
		({
			0, 1, 1, 2, 2, 3, 3, 0,  // top
			4, 0, 5, 1, 6, 2, 7, 3,  // belt
			4, 5, 5, 6, 6, 7, 7, 4,  // bottom
		});
		
	case Primitive::TRIANGLES:
		return std::vector<uint8_t>
		({
			1,5,2, 2,5,6,  // left
			3,7,0, 0,7,4,  // right
			2,6,3, 3,6,7,  // back
			0,4,1, 1,4,5,  // front
			6,5,7, 7,5,4,  // bottom
			1,2,0, 0,2,3,  // top
		});
/*
	case Primitive::TRIANGLE_STRIP:
		// glEnable(GL_PRIMITIVE_RESTART);  // OpenGL 3.1+
		// glPrimitiveRestartIndex(0xFF);
		return std::vector<uint8_t>
		({
			1, 5, 2, 6, 3, 7, 0, 4,  // left-back-right
			0xFF,  // This is a restart index
			2, 3, 1, 0, 5, 4, 6, 7,  // top-front-bottom
		});
		
	case Primitive::TRIANGLE_FAN:
		// Using triangle fan would be more fun!
		// input n vertices, output n - 2 triangles.
		// const size_t stride = sizeofArray(indices) >> 1;
		// glDrawElements(GL_TRIANGLE_FAN, stride - 2, GL_UNSIGNED_BYTE, indices);
		// glDrawElements(GL_TRIANGLE_FAN, stride - 2, GL_UNSIGNED_BYTE, indices + stride);
		return std::vector<uint8_t>
		({
			6, 7, 3, 2, 1, 5, 4, 7,  // 1st fan
			0, 1, 2, 3, 7, 4, 5, 1,  // 2nd fan
		});
*/
	case Primitive::QUADRILATERALS:
	case Primitive::POLYGON:
/*
		0---2      0---3
		|   |  =>  |   |
		1---3      1---2
		notice triangle_strip and quadrilateral's indice order.
*/
		return std::vector<uint8_t>
		({
			1, 5, 6, 2,  // left
			3, 7, 4, 0,  // right
			2, 6, 7, 3,  // back
			0, 4, 5, 1,  // front
			6, 5, 4, 7,  // bottom
			1, 2, 3, 0,  // top
		});
	
	default:
		return std::vector<uint8_t>{};
	}
}

std::vector<vec2f> Cube::getTexcoordData()
{
	std::vector<vec2f> texcoords(4);
	texcoords[0] = vec2f(0, 1);
	texcoords[1] = vec2f(0, 0);
	texcoords[2] = vec2f(1, 1);
	texcoords[3] = vec2f(1, 0);
	
	return texcoords;
}

/*
	all faces map to
	0-----2
	|     |
	|     |
	1-----3
*/
static const uint8_t quadrilateralIndex[4] =
{
	0, 1, 2, 3,
};

static const uint8_t triangleIndex[6] =
{
	0, 1, 2,  // upper left triangle
	2, 1, 3,  // lower right triangle
};

std::vector<uint8_t> Cube::getTexcoordIndex(Primitive primitive)
{
	std::vector<uint8_t> indices;
	
	switch(primitive)
	{
	case Primitive::TRIANGLES:
		{
			indices.resize(faceCount * sizeofArray(triangleIndex));
			uint8_t* data = indices.data();
			
			const uint8_t* end = triangleIndex + sizeofArray(triangleIndex);
			for(uint8_t f = 0; f < faceCount; ++f, data += sizeof(triangleIndex))
				std::copy(triangleIndex, end, data);
		}
		break;
	
	case Primitive::QUADRILATERALS:
		{
			indices.resize(faceCount * sizeofArray(quadrilateralIndex));
			uint8_t* data = indices.data();
			
			const uint8_t* end = quadrilateralIndex + sizeofArray(quadrilateralIndex);
			for(uint8_t f = 0; f < faceCount; ++f, data += sizeof(quadrilateralIndex))
				std::copy(quadrilateralIndex, end, data);
		}
		break;
	
	default:
		// wireframe doesn't use texture coordinate data.
		break;
	}
	
	return indices;
}

/*
	6-----7  1-----0
	|     |  |     |
	|     |  |     |  bottom | top
	5-----4  2-----3

	2-----3  0-----1
	|     |  |     |
	|     |  |     |  back | front
	6-----7  4-----5
	
	1-----2  3-----0
	|     |  |     |
	|     |  |     |  left | right
	5-----6  7-----4
*/
std::vector<vec2f> Cube::getPackedTexcoordData(Primitive primitive)
{
	if(primitive != Primitive::TRIANGLES && primitive != Primitive::QUADRILATERALS)
		return {};
	
	std::vector<vec2f> texcoords;
	
	constexpr float edgeLength = 1.0 / 3;
	for(uint8_t f = 0; f < faceCount; ++f)
	{
		float s = (f & 1) == 0? 0: edgeLength;
		float t = (f >> 1) * edgeLength; 
		vec2f origin = vec2f(s, t);
		
		vec2f _0(origin.s, origin.t + edgeLength);
		vec2f& _1 = origin;
		vec2f _2 = vec2f(origin.s + edgeLength, origin.t + edgeLength);
		vec2f _3 = vec2f(origin.s + edgeLength, origin.t);
		
		texcoords.push_back(_0);
		texcoords.push_back(_1);
		texcoords.push_back(_2);
		
		if(primitive == Primitive::TRIANGLES)
		{
			texcoords.push_back(_2);
			texcoords.push_back(_1);
		}
		
		texcoords.push_back(_3);
	}
	
	return texcoords;
}

std::vector<vec3f> Cube::getNormalData()
{
	std::vector<vec3f> normals(faceCount, vec3f(0, 0, 0));
	for(uint8_t f = 0; f < faceCount; ++f)
		normals[f][f / 2] = (f % 2 == 0)? -1.0F: 1.0F; 
	
	return normals;
}

std::vector<uint8_t> Cube::getNormalIndex(Primitive primitive)
{
	std::vector<uint8_t> indices;
	
	switch(primitive)
	{
	case Primitive::TRIANGLES:
	case Primitive::QUADRILATERALS:
		{
			const uint8_t n = (primitive == Primitive::QUADRILATERALS)? 4: 6;
			indices.reserve(faceCount * n);
			for(uint8_t f = 0; f < faceCount; ++f)
				for(uint8_t i = 0; i < n; ++i)
					indices.push_back(f);
		}
		break;
	
	default:
		break;
	}
	
	return indices;
}

