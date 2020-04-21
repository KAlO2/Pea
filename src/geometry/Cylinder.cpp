#include "geometry/Cylinder.h"


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

uint32_t Cylinder::slice = 32;
CapFillType Cylinder::cap = CapFillType::TRIANGLE_FAN;

void Cylinder::setSlice(uint32_t slice)
{
	assert(slice >= 3U);
	Cylinder::slice = slice;
}

size_t Cylinder::getVertexSize()
{
	return (slice + (cap == CapFillType::TRIANGLE_FAN)) << 1;
}

std::vector<vec3f> Cylinder::getVertexData() const
{
	size_t size = getVertexSize();
	std::vector<vec3f> vertices;
	vertices.reserve(size);
	const float z = height / 2;
	for(uint32_t i = 0; i < slice; ++i)
	{
		double angle = (i * (2 * M_PI)) / slice;
		double x = radius * std::cos(angle);
		double y = radius * std::sin(angle);
		vertices.emplace_back(x, y, +z);
		vertices.emplace_back(x, y, -z);
	}
	
	if(cap == CapFillType::TRIANGLE_FAN)
	{
		vertices.emplace_back(0.0F, 0.0F, +z);
		vertices.emplace_back(0.0F, 0.0F, -z);
	}
	
	assert(vertices.size() == size);
	return vertices;
}

size_t Cylinder::getIndexSize(Primitive primitive)
{
	// slice * 2 + 2 + cap * ((slice + 1) * 4 + 2)
	// slice * 2 + 2 + (slice + 1) * 4 + 2
	// slice *6 + 8
	switch(primitive)
	{
	case Primitive::POINTS:
		return (slice + (cap == CapFillType::TRIANGLE_FAN)) << 1;
	
	case Primitive::LINES:
		return slice * (cap == CapFillType::TRIANGLE_FAN? 5: 3) * 2;

	case Primitive::LINE_STRIP:
		assert(cap == CapFillType::TRIANGLE_FAN);
		// 1 + (slice + 1)*2 + 1 + 3 * (slice - 1)
		return slice * 5 + 1; 

	case Primitive::TRIANGLES:
		return slice * (cap == CapFillType::NONE? 6: 12);
	
	case Primitive::TRIANGLE_STRIP:
		return /*(cap == CapFillType::TRIANGLE_FAN)? (slice * 6 + 8):*/ ((slice + 1) << 1);
	
	case Primitive::TRIANGLE_FAN:
		// top and bottom, 2 fans
		return (cap == CapFillType::TRIANGLE_FAN)? ((2 + slice) << 1): 0;
	
	case Primitive::QUADRILATERAL_STRIP:
		// only belt part
		return (slice + 1) << 1;
	
	case Primitive::POLYGON:
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
	
	const uint32_t topIndex = slice << 1, bottomIndex = topIndex + 1;
	switch(primitive)
	{
	case Primitive::POINTS:
		for(uint32_t i = 0, vertexSize = getVertexSize(); i < vertexSize; ++i)
			indices.push_back(i);
		break;
	
	case Primitive::LINES:
/*
		      /  topIndex
		    j0/___j1
		    |
		    |
		j0+1|____j1+1
		    \
		     \  bottomIndex
*/
		for(uint32_t i = 0; i < slice; ++i)
		{
			uint32_t j0 = i << 1;
			uint32_t j1 = ((i + 1) % slice) << 1;
			
			indices.push_back(j0);
			indices.push_back(j0 + 1);
			
			indices.push_back(j0);
			indices.push_back(j1);
			
			indices.push_back(j0 + 1);
			indices.push_back(j1 + 1);
			
			if(cap == CapFillType::TRIANGLE_FAN)
			{
				indices.push_back(j0);
				indices.push_back(topIndex);
				
				indices.push_back(j0 + 1);
				indices.push_back(bottomIndex);
			}
		}
		break;

	case Primitive::LINE_STRIP:
		// According to graph theory, NONE and POLYGON can't be finished in one line strip.
		assert(cap == CapFillType::TRIANGLE_FAN);
		indices.push_back(topIndex);
		
		// top loop edge
		for(uint32_t i = 0; i < slice; ++i)
			indices.push_back(i << 1);
		indices.push_back(0);
		
		// bottom loop edge
		for(uint32_t i = 0; i < slice; ++i)
			indices.push_back((i << 1) + 1);
		indices.push_back(1);
		
		indices.push_back(bottomIndex);
		for(uint32_t i = 1; i < slice; ++i)
		{
			if((i & 1) != 0)
			{
				indices.push_back((i << 1) + 1);
				indices.push_back(i << 1);
				indices.push_back(topIndex);
			}
			else
			{
				indices.push_back(i << 1);
				indices.push_back((i << 1) + 1);
				indices.push_back(bottomIndex);
			}
		}
		break;

	case Primitive::TRIANGLES:
		{
			for(uint32_t i = 0; i < topIndex; i += 2)
			{
				uint32_t i1 = i + 1;
				uint32_t i2 = (i + 2) % topIndex;
				uint32_t i3 = i2 + 1;
				
				indices.push_back(i);
				indices.push_back(i1);
				indices.push_back(i2);
				
				indices.push_back(i2);
				indices.push_back(i1);
				indices.push_back(i3);
			}
			
			assert(cap != CapFillType::NGON);
			if(cap != CapFillType::TRIANGLE_FAN)
			{
				for(uint32_t i = 1; i < topIndex; i += 2)
				{
					indices.push_back(i);
					indices.push_back(bottomIndex);
					indices.push_back((i + 2) % topIndex);
				}

				for(uint32_t i = 0; i < topIndex; i += 2)
				{
					indices.push_back(topIndex);
					indices.push_back(i);
					indices.push_back((i + 2) % topIndex);
				}
			}
		}
		break;
	
	case Primitive::TRIANGLE_STRIP:
		for(uint32_t i = 0; i < topIndex; ++i)
			indices.push_back(i);
		indices.push_back(0);
		indices.push_back(1);
		break;
		
	case Primitive::TRIANGLE_FAN:
		// top and bottom fans
		indices.push_back(topIndex);
		for(uint32_t i = 0; i < topIndex; i += 2)
			indices.push_back(i);
		indices.push_back(0);  // forms top circle
//		indices.push_back(0xFFFFFFFF);  // primitive restart?
		indices.push_back(bottomIndex);
		for(uint32_t i = 1; i < topIndex; i += 2)
			indices.push_back(i);
		indices.push_back(1);  // forms bottom circle
		break;
	
	case Primitive::POLYGON:
		// return two polygons, top and bottom face.
		{
			assert(cap == CapFillType::NGON);
			for(uint32_t i = 0; i < slice; ++i)
				indices.push_back(i << 1);
			for(uint32_t i = slice - 1; i-- > 0; --i)
				indices.push_back((i << 1) + 1);
		}
		break;
	
	default:
		break;
	}
	
	assert(indices.size() == size);
	return indices;
}

std::vector<vec3f> Cylinder::getNormalData()
{
	size_t size = slice + (cap == CapFillType::TRIANGLE_FAN);
	std::vector<vec3f> normals;
	normals.reserve(size);
	
	for(uint32_t i = 0; i < slice; ++i)
	{
		double angle = (i * (2 * M_PI)) / slice;
		double x = std::cos(angle);
		double y = std::sin(angle);
		normals.emplace_back(x, y, 0.0);
	}
	
	if(cap == CapFillType::TRIANGLE_FAN)
	{
		normals.emplace_back(0.0, 0.0, +1.0);
		normals.emplace_back(0.0, 0.0, -1.0);
	}
	
	assert(normals.size() == size);
	return normals;
}

std::vector<uint32_t> Cylinder::getNormalIndex(Primitive primitive)
{
	size_t size = getIndexSize(primitive);
	std::vector<uint32_t> indices;
	indices.reserve(size);
	
	switch(primitive)
	{
	case Primitive::TRIANGLES:
		{
			for(uint32_t i = 0; i < slice; ++i)
			{
				uint32_t i1 = (i + 1) % slice;
				
				indices.push_back(i);
				indices.push_back(i);
				indices.push_back(i1);
				
				indices.push_back(i1);
				indices.push_back(i);
				indices.push_back(i1);
			}
			
			assert(cap != CapFillType::NGON);
			if(cap == CapFillType::TRIANGLE_FAN)
			{
				const uint32_t UP = slice, DOWN = slice + 1;
				const int32_t count = 3 * slice;
				for(int32_t i = 0; i < count; ++i)
					indices.push_back(UP);
				for(int32_t i = 0; i < count; ++i)
					indices.push_back(DOWN);
			}
		}
		break;
		
	case Primitive::TRIANGLE_STRIP:
		for(uint32_t i = 0; i < slice; ++i)
		{
			indices.emplace_back(i);
			indices.emplace_back(i);
		}
		indices.emplace_back(0);
		indices.emplace_back(0);
		break;
		
	case Primitive::TRIANGLE_FAN:
	case Primitive::POLYGON:
		assert((primitive == Primitive::TRIANGLE_FAN && cap == CapFillType::TRIANGLE_FAN) ||
				(primitive == Primitive::POLYGON && cap == CapFillType::NGON));
		{
			const uint32_t UP = slice, DOWN = slice + 1;
			uint32_t count = slice + (primitive == Primitive::TRIANGLE_FAN);
			for(uint32_t i = 0; i < count; ++i)
				indices.emplace_back(DOWN);
			for(uint32_t i = 0; i < count; ++i)
				indices.emplace_back(UP);
		}
		break;
	
	default:
		break;
	}
	
	assert(indices.size() == size);
	return indices;
}

static void appendTexcoordData(std::vector<vec2f>& texcoords, uint32_t slice, float x, float y, float radius, bool centerFirst)
{
	for(uint32_t i = 0; i <= slice; ++i)
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
		for(uint32_t i = 0; i <= slice; ++i)
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
