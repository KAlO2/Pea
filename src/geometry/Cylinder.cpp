#include "geometry/Cylinder.h"

#include "geometry/Grid.h"

using namespace pea;

Cylinder::Cylinder(float radius, float height):
		radius(radius),
		height(height)
{
	assert(radius > 0 && height > 0);
}

vec3f Cylinder::ineria(float mass)
{
	float Iz = radius * radius / 2;
	float Iy = Iz / 2 + height * height / 12;
	float&Ix = Iy;

	Ix *= mass;
	Iy *= mass;
	return vec3f(Ix, Iy, Iz);
}

int32_t Cylinder::slice = 32;
int32_t Cylinder::stack = 1;
CapFillType Cylinder::cap = CapFillType::TRIANGLE_FAN;

void Cylinder::setSlice(int32_t slice)
{
	assert(slice >= 3);
	Cylinder::slice = slice;
}

void Cylinder::setStack(int32_t stack)
{
	assert(stack > 0);
	Cylinder::stack = stack;
}

size_t Cylinder::getVertexSize()
{
	size_t size = slice * (stack + 1);
	// triangle fan needs top center and bottonm center two vertices, ngon doesn't need.
	if(cap == CapFillType::TRIANGLE_FAN)
		size += 2;
	return size;
}

std::vector<vec3f> Cylinder::getVertexData() const
{
	size_t size = getVertexSize();
	std::vector<vec3f> vertices;
	vertices.reserve(size);
	
	std::vector<vec2f> table;
	table.reserve(slice);
	for(int32_t i = 0; i < slice; ++i)
	{
		double angle = ((2 * M_PI) * i) / slice;
		float x = static_cast<float>(radius * std::cos(angle));
		float y = static_cast<float>(radius * std::sin(angle));
		table.emplace_back(x, y);
	}
	
	const bool hasCap = cap == CapFillType::TRIANGLE_FAN;
	if(hasCap)
		vertices.emplace_back(0.0F, 0.0F, -height / 2);  // bottom center
	
	const float q = height / 4;
	for(int32_t j = 0; j <= stack; ++j)
	{
		float z = ((j << 1) - stack) * q;
		for(const vec2f& p: table)
			vertices.emplace_back(p.x, p.y, z);
	}
	
	if(hasCap)
		vertices.emplace_back(0.0F, 0.0F, +height / 2);  // top center
	
	assert(vertices.size() == size);
	return vertices;
}

size_t Cylinder::getIndexSize(Primitive primitive)
{
	switch(primitive)
	{
	case Primitive::POINTS:
		return getVertexSize();
	
	case Primitive::LINES:
		// line number is (stack * 2 + 3) * slice, see LINE_STRIP below for explanation.
		// each line consists of two end points.
		return (stack * 2 + 1 + (cap == CapFillType::TRIANGLE_FAN) * 2) * slice * 2;

	case Primitive::LINE_STRIP:
		// According to graph theory, NONE and POLYGON can't be finished in one line strip.
		assert(cap == CapFillType::TRIANGLE_FAN);
		// (stack + 2) * slice + (stack + 1) * slice + 1;
		// strip from top to bottom center per slice -> stack + 2
		// stack edges per slice-> stack + 1
		// extra 1 -> n line segments have n + 1 points.
		return (stack * 2 + 3) * slice + 1;
	
	case Primitive::TRIANGLES:
		assert(cap != CapFillType::POLYGON);
		{
			// (bottom tri + quad + top tri) per slice
			size_t size = Grid::getIndexSize(slice, stack, primitive);
			if(cap == CapFillType::TRIANGLE_FAN)
				size += slice * 3 * 2;
			return size;
		}
	
	case Primitive::TRIANGLE_STRIP:
//		return /*(cap == CapFillType::TRIANGLE_FAN)? (slice * 6 + 8):*/ ((slice + 1) << 1);
		// TRIANGLE_FAN can be seen as two more stacks.
		return Grid::getIndexSize(slice, stack + (cap == CapFillType::TRIANGLE_FAN) * 2, primitive);
	
	case Primitive::TRIANGLE_FAN:
		// bottom and top, 2 fans
		if(cap == CapFillType::TRIANGLE_FAN)
			return (2 + slice) * 2;
		else
			return 0;
	
	case Primitive::QUADRILATERAL_STRIP:
		// only belt part
		return slice * stack * 4;
	
	case Primitive::POLYGON:
		assert(cap == CapFillType::POLYGON);
		// top and bottom, 2 polygons
		return slice << 1;
		
	default:
		return 0;
	}
}

std::vector<uint32_t> Cylinder::getVertexIndex(Primitive primitive)
{
	std::vector<uint32_t> indices;
	size_t size = getIndexSize(primitive);
	indices.reserve(size);
	
	const int32_t bottomIndex = 0, topIndex = (stack + 1) * slice + 1;
	const bool hasCap = cap == CapFillType::TRIANGLE_FAN;
	switch(primitive)
	{
	case Primitive::POINTS:
		for(uint32_t i = 0, vertexSize = getVertexSize(); i < vertexSize; ++i)
			indices.push_back(i);
		break;
	
	case Primitive::LINES:
		// Note that you can get answer from #lineStripToLines() function.
		// This approach generate indices from bottom to top.
		for(int32_t k = 1; k <= slice; ++k)
		{
			indices.push_back(bottomIndex);
			indices.push_back(k);
		}
		
		for(int32_t j = 0; j <= stack; ++j)
		{
			const uint32_t base = 1 + j * slice;
			for(int32_t i = 0; i <= slice; ++i)
			{
				indices.push_back(base + i);
				indices.push_back(base + (i + 1) % slice);
				
				if(j < stack)
				{
					indices.push_back(base + i);
					indices.push_back(base + i + slice);
				}
			}
		}
		
		for(int32_t k = topIndex - slice; k < topIndex; ++k)
		{
			indices.push_back(k);
			indices.push_back(topIndex);
		}
		break;

	case Primitive::LINE_STRIP:
		// According to graph theory, NONE and POLYGON can't be finished in one line strip.
		assert(cap == CapFillType::TRIANGLE_FAN);
		indices.push_back(bottomIndex);
		
		// each stack around
		for(int32_t j = 0; j <= stack; ++j)
		{
			int32_t base = 1 + j * slice;
			for(int32_t i = 0; i < slice; ++i)
				indices.push_back(base + i);
			indices.push_back(base);
		}
		
		indices.push_back(topIndex);
		
		// now comes to slice, the first slice has been drawn, we'll start from the second slice.
		for(int32_t i = 1, base = 1 + stack * slice; i < slice; ++i)
		{
			if((i & 1) != 0)  // odd, start from topIndex
			{
				for(int32_t k = base + i; k > 0; k -= slice)
					indices.push_back(k);
				indices.push_back(bottomIndex);
			}
			else  // even, start from bottomIndex
			{
				for(int32_t k = 1 + i; k < topIndex; k += slice)
					indices.push_back(k);
				indices.push_back(topIndex);
			}
		}
		break;

	case Primitive::TRIANGLES:
		if(hasCap)  // bottom face
		{
			for(int32_t i = slice; i > 0; --i)
			{
				indices.push_back(bottomIndex);
				indices.push_back(i % slice + 1);
				indices.push_back(i);
			}
		}
		
		for(int32_t j = 0; j < stack; ++j)
		{
			int32_t base = 1 + j * slice;
			for(int32_t i = 0; i < slice; ++i)
			{
				int32_t _2 = base + slice + i, _3 = base + slice + (i + 1) % slice;  // _2, _3
				int32_t _0 = _2 - slice,       _1 = _3 - slice;                      // _0, _1
				
				indices.push_back(_2);
				indices.push_back(_0);
				indices.push_back(_3);
				
				indices.push_back(_3);
				indices.push_back(_0);
				indices.push_back(_1);
			}
		}
		
		if(hasCap)  // top face
		{
			const uint32_t base = topIndex - slice;
			for(int32_t i = base; i < topIndex; ++i)
			{
				indices.push_back(topIndex);
				indices.push_back(i);
				indices.push_back(i + 1 < topIndex? i + 1: base);
			}
		}
		break;
	
	case Primitive::TRIANGLE_STRIP:
		if(hasCap)  // bottom face
		{
			for(int32_t i = 1; i <= slice; ++i)
			{
				indices.push_back(i);
				indices.push_back(bottomIndex);
			}
		
			// form a loop
			indices.push_back(1);
			indices.push_back(bottomIndex);
			// degenerated
			indices.push_back(bottomIndex);  // previous
			indices.push_back(slice + 1);  // next
		}
		
		for(int32_t j = 0; j < stack; ++j)
		{
			int32_t base = hasCap + j * slice;
			for(int32_t i = 0; i < slice; ++i)
			{
				indices.push_back(base + i + slice);
				indices.push_back(base + i);
			}
			
			// form a loop
			indices.push_back(base + slice);
			indices.push_back(base);
			if(hasCap || j != stack - 1)  // degenerated
			{
				indices.push_back(base);  // previous
				indices.push_back(base + slice * 2);  // next
			}
		}
		
		if(hasCap)  // top face
		{
			for(int32_t i = topIndex - slice; i < topIndex; ++i)
			{
				indices.push_back(topIndex);
				indices.push_back(i);
			}
			
			// form a loop
			indices.push_back(topIndex);
			indices.push_back(topIndex - slice);
			// at end, no need to degenerate.
		}
		break;
		
	case Primitive::TRIANGLE_FAN:
		// bottom and top fans
		assert(cap == CapFillType::TRIANGLE_FAN);
		{
			indices.push_back(bottomIndex);
			indices.push_back(1);
			for(int32_t i = slice; i > 0; --i)
				indices.push_back(i);
			
//			indices.push_back(0xFFFFFFFF);  // TODO: primitive restart?
			
			indices.push_back(topIndex);
			for(int32_t i = topIndex - slice; i < topIndex; ++i)
				indices.push_back(i);
			indices.push_back(topIndex - slice);
		}
		break;
	
//	case Primitive::QUADRILATERAL_STRIP:
	
	case Primitive::POLYGON:
		// return two polygons, bottom and top face.
		assert(cap == CapFillType::POLYGON);
		{
			// notice the bottom winding
			indices.push_back(0);
			for(int32_t i = slice - 1; i > 0; --i)
				indices.push_back(i);
			
//			indices.push_back(0xFFFFFFFF);  // TODO: primitive restart?
			int32_t base = stack * slice;
			for(int32_t i = 0; i < slice; ++i)
				indices.push_back(base + i);
		}
		break;
	
	default:
		break;
	}
	
	assert(indices.size() == size);
	return indices;
}

//std::vector<uint32_t> Cylinder::getTubeVertexIndex(uint32_t slice, uint32_t loopCut)

std::vector<vec3f> Cylinder::getNormalData()
{
	const bool hasCap = cap != CapFillType::NONE;
	size_t size = slice;
	if(hasCap)
		size += 2;  // top and bottom face normal
	
	std::vector<vec3f> normals;
	normals.reserve(size);
	
	for(int32_t i = 0; i < slice; ++i)
	{
		double angle = (i * (2 * M_PI)) / slice;
		double x = std::cos(angle);
		double y = std::sin(angle);
		normals.emplace_back(x, y, 0.0);
	}
	
	if(hasCap)
	{
		normals.emplace_back(0.0, 0.0, +1.0);  // top normal
		normals.emplace_back(0.0, 0.0, -1.0);  // bottom normal
	}
	
	assert(normals.size() == size);
	return normals;
}

std::vector<uint32_t> Cylinder::getNormalIndex(Primitive primitive)
{
	size_t size = getIndexSize(primitive);
	std::vector<uint32_t> indices;
	indices.reserve(size);
	
	const int32_t UP = slice, DOWN = slice + 1;
	switch(primitive)
	{
	case Primitive::TRIANGLES:
		assert(cap != CapFillType::POLYGON);
		if(cap == CapFillType::TRIANGLE_FAN)
		{
			const int32_t count = 3 * slice;
			for(int32_t i = 0; i < count; ++i)
				indices.push_back(UP);
		}
		
		for(int32_t i = 0; i < slice; ++i)
		{
			int32_t i1 = (i + 1) % slice;
			
			indices.push_back(i);
			indices.push_back(i);
			indices.push_back(i1);
			
			indices.push_back(i1);
			indices.push_back(i);
			indices.push_back(i1);
		}
		
		if(cap == CapFillType::TRIANGLE_FAN)
		{
			const int32_t count = 3 * slice;
			for(int32_t i = 0; i < count; ++i)
				indices.push_back(DOWN);
		}
		break;
		
	case Primitive::TRIANGLE_STRIP:
		for(int32_t i = 0; i < slice; ++i)
		{
			indices.emplace_back(i);
			indices.emplace_back(i);
		}
		indices.emplace_back(0);
		indices.emplace_back(0);
		break;
		
	case Primitive::TRIANGLE_FAN:
		assert(cap == CapFillType::TRIANGLE_FAN);
		// center and loop, extra two points.
		for(int32_t i = 0; i <= (slice + 1); ++i)
			indices.emplace_back(UP);
		for(int32_t i = 0; i < (slice + 1); ++i)
			indices.emplace_back(DOWN);
		break;
		
	case Primitive::POLYGON:
		assert(cap == CapFillType::POLYGON);
		for(int32_t i = 0; i < slice; ++i)
			indices.emplace_back(DOWN);
		for(int32_t i = 0; i < slice; ++i)
			indices.emplace_back(UP);
		break;
	
	default:
		break;
	}
	
	assert(indices.size() == size);
	return indices;
}

static void appendTexcoordData(std::vector<vec2f>& texcoords, int32_t slice, float x, float y, float radius, bool centerFirst)
{
	for(int32_t i = 0; i <= slice; ++i)
	{
		double angle = (i * (2 * M_PI)) / slice;
		double dx = radius * std::cos(angle);
		double dy = radius * std::sin(angle);
		if(centerFirst)
		{
			texcoords.emplace_back(x, y);
			texcoords.emplace_back(x + dx, y + dy);
		}
		else
		{
			texcoords.emplace_back(x + dx, y + dy);
			texcoords.emplace_back(x, y);
		}
	}
}

std::vector<vec2f> Cylinder::getTexcoordData(Primitive primitive) const
{
	std::vector<vec2f> texcoords;
	
	const float l = 2 * M_PI * radius;
	const float t = std::min(height / l, 1.0F);
	float bandWidth = std::min(l, height) / slice;
	switch(primitive)
	{
	case Primitive::TRIANGLES: {
	
	
		} break;
	
	case Primitive::TRIANGLE_FAN:
		appendTexcoordData(texcoords, slice, 0.5, 0.5, 0.5, true);
		appendTexcoordData(texcoords, slice, 1.5, 0.5, 0.5, true);
		break;
	
	case Primitive::QUADRILATERAL_STRIP:
		// belt
		for(int32_t i = 0; i <= slice; ++i)
		{
			float s = i * bandWidth;
			texcoords.emplace_back(s, t);
			texcoords.emplace_back(s, 0);
		}
		break;
	
	default:
		break;
	}
/*
	
	if(cap)
	{
		size_t size = getIndexSize(primitive);
		texcoords.reserve(size);
	
		// two circles horizontal up: radius * 2 + height <= l
		// two circles vertical right: l + 2 * radius <= height
		// otherwise, right, scale to fit
		if(radius * 2 + height <= l)
		{
			fillHorizontal();
			
			float y = (height + radius) / l;
			radius /= l;
			
			putCircle(radius, y, radius, false);
			texcoords.emplace_back(radius * 2, y);
			texcoords.emplace_back(radius * 4, y);
			putCircle(radius * 3, y, radius, true);
		}
		else // if(l + 2 * radius <= height)
		{
			fillVertical();
			
			float x = (l + radius) / height;
			radius /= height;
			
			putCircle(x, radius, radius, false);
			texcoords.emplace_back(x, radius);
			texcoords.emplace_back(x, radius * 3);
			putCircle(x, radius * 3, radius, true);
			
			if(l + 2 * radius > height)
			{
				const float s = height / (l + 2 * radius);
				for(vec2f& texcoord: texcoords)
					texcoord *= s;
			}
		}
	}
	else
	{
		texcoords.reserve((slice + 1) << 1);
		if(l >= height)
			fillHorizontal();
		else  // l < height
			fillVertical();
	}
*/
	return texcoords;
}
