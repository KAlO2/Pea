#include "geometry/Primitive.h"

#include <cassert>

namespace pea {

std::vector<uint32_t> lineStripToLines(const uint32_t* strip, size_t length)
{
//	assert(length >= 2);
	if(length < 2)
		return {};
	
	std::vector<uint32_t> lines;
	lines.reserve((length - 1) << 1);
	
	for(size_t i = 1; i < length; ++i)
	{
		lines.push_back(strip[i - 1]);
		lines.push_back(strip[i]);
	}
	
	return lines;
}

std::vector<uint32_t> lineLoopToLines(const uint32_t* loop, size_t length)
{
//	assert(length >= 3);
	if(length < 3)
		return {};
	
	std::vector<uint32_t> lines;
	lines.reserve(length << 1);
	
	
	for(size_t i0 = length - 1, i1 = 0; i1 < length; i0 = i1, ++i1)
	{
		lines.push_back(loop[i0]);
		lines.push_back(loop[i1]);
	}
	
	return lines;
}

std::vector<uint32_t> triangleStripToTriangles(const uint32_t* strip, size_t length)
{
//	assert(length >= 3);
	if(length < 3)
		return {};
	
	std::vector<uint32_t> triangles;
	triangles.reserve((length >> 1) * 3);
	
	bool direction = true;
	uint32_t i0 = strip[0];
	uint32_t i1 = strip[1];
	for(size_t i = 2; i < length; ++i)
	{
		const uint32_t& i2 = strip[i];
		if(i0 != i1 && i1 != i2 && i2 != i0)
		{
			if(direction)
			{
				triangles.push_back(i0);
				triangles.push_back(i1);
			}
			else
			{
				triangles.push_back(i1);
				triangles.push_back(i0);
			}
			
			triangles.push_back(i2);
		}
		
		i0 = i1;
		i1 = i2;
		direction = !direction;
	}
	
	return triangles;
}

std::vector<uint32_t> triangleFanToTriangles(const uint32_t* fan, size_t length)
{
//	assert(length >= 3);
	if(length < 3)
		return {};
	
	std::vector<uint32_t> triangles;
	triangles.reserve((length - 2) * 3);
	
	for(size_t i = 2; i < length; ++i)
	{
		triangles.push_back(fan[0]);
		triangles.push_back(fan[i - 1]);
		triangles.push_back(fan[i]);
	}
	
	return triangles;
}

std::vector<uint32_t> quadrilateralsToTriangles(const uint32_t* quad, size_t length)
{
	assert((length & 3) == 0);
	
	std::vector<uint32_t> triangles;
	triangles.reserve((length - 2) * 3);
	
	for(size_t i = 0; i < length; i += 4)
	{
		triangles.push_back(quad[i]);
		triangles.push_back(quad[i + 1]);
		triangles.push_back(quad[i + 2]);
		
		triangles.push_back(quad[i]);
		triangles.push_back(quad[i + 2]);
		triangles.push_back(quad[i + 3]);
	}
	
	return triangles;
}

std::vector<uint32_t> polygonsToTriangles(const uint32_t* polygon, const uint32_t* vertexSizes, size_t count)
{
	std::vector<uint32_t> triangles;
	
//	uint32_t sum = std::accumulate(vertexSizes, vertexSizes + count, 0);
	uint32_t n = 0;
	for(size_t i = 0; i < count; ++i)
	{
		assert(vertexSizes[i] >= 3);
		n += vertexSizes[i];
	}
	// polygon with n vertices => (n - 2) triangles.
	n = (n - count * 2) * 3;
	triangles.reserve(n);
	
	n = 0;
	for(size_t i = 0; i < count; ++i)
	{
		const uint32_t& v0 = polygon[n];
		uint32_t v1 = polygon[n + 1];
		for(uint32_t j = 2; j < vertexSizes[i]; ++j)
		{
			const uint32_t& v2 = polygon[n + j];
			triangles.push_back(v0);
			triangles.push_back(v1);
			triangles.push_back(v2);
			
			v1 = v2;
		}
		
		n += vertexSizes[i];
	}
	
	return triangles;
}

}  // namespace pea
