#include "geometry/Sphere.h"

#include "math/function.h"

using namespace pea;


Sphere::Sphere():
		center(vec3f(0.0)),
		radius(1.0)
{
}

Sphere::Sphere(const vec3f& center, float radius):
		center(center),
		radius(radius)
{
	assert(radius > 0);
}

Sphere& Sphere::operator=(const Sphere& other)
{
	if(this != &other)
	{
		center = other.center;
		radius = other.radius;
	}
	return *this;
}

bool Sphere::intersect(const Sphere& other)
{
	vec3f d0 = other.center - center;
	float sr = other.radius + radius;
	return d0.length2() < sr * sr;
}

void Sphere::merge(const Sphere &other)
{
	const vec3f D(other.center - center);
	float dd = dot(D, D);
	float diff = other.radius - radius;

	// one fully contains the other one
	if(diff * diff >= dd)
	{
		if(diff > 0)
		{
			center = other.center;
			radius = other.radius;
		}
		return;
	}
/*
	realm0 = C0 - r0 * normalize(C0C1);
	realm1 = C1 + r1 * normalize(C0C1);
	center = (realm0 + realm1)/2;
	radius = (r0 + r1 + C0C1)/2
*/
	float d = std::sqrt(dd);
	center = (center + other.center + diff / d * D)/2;
	radius = (radius + d + other.radius)/2;
}

bool Sphere::intersect(const Sphere& lhs, const Sphere& rhs)
{
	const vec3f D = lhs.center - rhs.center;
	float a = lhs.radius + rhs.radius;
	return dot(D, D) < a * a;  // D.length() < a
}

vec3f Sphere::composeOrientation(const float& pitch, const float& yaw)
{
	// use double precision
	double _pitch = pitch, _yaw = yaw;
	double cos_pitch = std::cos(_pitch), cos_yaw = std::cos(_yaw);
	double sin_pitch = std::sin(_pitch), sin_yaw = std::sin(_yaw);
#if 1//USE_MATH_XYZ
	vec3f forward(cos_pitch * cos_yaw, cos_pitch * sin_yaw, sin_pitch);
#else
	vec3f forward(cos_pitch * cos_yaw, sin_pitch, cos_pitch * sin_yaw);
#endif
	return forward;
}

std::pair<float, float> Sphere::decomposeOrientation(const vec3f& forward)
{
	// forward.length2() == 1
//	assert(std::abs(forward.length() - 1) <= std::numeric_limits<float>::epsilon());
	
#if 1//USE_MATH_XYZ
	constexpr int8_t X = 0, Y = 1, Z = 2;
#else
	constexpr int8_t X = 0, Z = 1, Y = 2;
#endif
	const float& sin_pitch = forward[Z];
	assert(std::abs(sin_pitch) <= 1.0F);
	float pitch = std::asin(sin_pitch);
	
	float cos_pitch = std::sqrt(1 - sin_pitch * sin_pitch);
	float cos_yaw = forward[X] / cos_pitch;
//	float sin_yaw = forward[Y] / cos_pitch;

	float yaw = std::acos(cos_yaw);  // (0, pi)
	// pitch in [-pi/2, pi/2], cos_pitch >= 0, so sin_yaw and forward[Y] have the same sign.
	if(forward[Y] <= 0)  // same as if(sin_yaw > 0), mirror yaw to interval [-pi, 0]
		yaw = -yaw;
	
	return std::make_pair(pitch, yaw);
}

bool Sphere::wrap(float& pitch, float& yaw)
{
	if(std::abs(pitch) < M_PI / 2)
		return false;
	
	// pitch constrains to [-pi/2, pi/2]
	if(pitch > M_PI / 2)
		pitch = M_PI - pitch;
	else
		pitch = -M_PI - pitch;
	
	// go to the reverse side, with M_PI radians shift.
	if(yaw > M_PI)
		yaw -= M_PI;
	else if(yaw < -M_PI)
		yaw += M_PI;
	
	return true;
}

// default parameters used in Blender
uint32_t Sphere::slice = 32;
uint32_t Sphere::stack = 16;
uint32_t Sphere::seam  =  0;

void Sphere::setSlice(uint32_t slice)
{
	assert(slice >= 3U);
	Sphere::slice = slice;
}

void Sphere::setStack(uint32_t stack)
{
	assert(stack >= 2U);
	Sphere::stack = stack;
}

void Sphere::setSeam(uint32_t seam)
{
	if(seam <= 0 || seam >= Sphere::stack)
		seam = 0;
	if(seam > Sphere::stack / 2)
		seam = Sphere::stack - seam;
	Sphere::seam = seam;
}

size_t Sphere::getVertexSize()
{
	assert(seam <= stack / 2);
	if(seam == 0)
		return slice * (stack - 1) + 2;  // bottom 1, plus top 1.
	else
	{
		size_t size = slice * (stack + 1) + 2;
		if(seam / 2 != 0)  // odd
			size += stack - seam;
		else if(seam * 2 != stack) // even, seam not on the equator
			size += stack - seam * 2 + 1;
//		else  // special case for even number
//			size += 0;
		return size;
	}
}

std::vector<vec3f> Sphere::getVertexData() const
{
	std::vector<vec3f> data = getNormalData();
	// for unit sphere, normal data can serve as vertex data directly.
	if(center != vec3f(0, 0, 0) || radius != 1.0F)
		for(vec3f& datum: data)
			datum = center + radius * datum;
	
	return data;
}

size_t Sphere::getVertexIndexSize(Primitive primitive)
{
	switch(primitive)
	{
	case Primitive::POINTS:
		return getVertexSize();
	
	case Primitive::LINES:
		if(seam == 0)// slice * 2 + ((stack - 2 + (stack - 1) * 2) * slice
			return (3 * stack - 2) * slice;
		else
			return 0;
	
	case Primitive::LINE_STRIP:
		assert(seam == 0);  // seam > 0 forms 2 or 3 line strips.
		// 2 + (stack - 1) * (slice + 1) + stack * (slice - 1)
		return slice * stack * 2 - slice + 1;
	
	case Primitive::TRIANGLES:
		// slice * 3 * 2 + slice * (stack - 2) * 6;
		return slice * (stack - 1) * 6;
	
	case Primitive::TRIANGLE_STRIP:
		// uint32_t indexPerStack = (slice + 1) << 1;
		return stack * (slice + 1) << 1;
	
	case Primitive::TRIANGLE_FAN:
		return (slice + 2) << 1;
	
	case Primitive::QUADRILATERALS:
		return (stack - 2) * slice * 4;
	
	default:
		return 0;
	}
}

void Sphere::pushLineStripIndexData(std::vector<uint32_t>& indices, const size_t& vertexSize)
{
	// add each stack from top to bottom.
	indices.emplace_back(0);
	for(uint32_t j = 0; j < stack - 1; ++j)
	{
		uint32_t base = 1 + j * slice;
		for(uint32_t i = 0; i < slice; ++i)
			indices.emplace_back(base + i);
		indices.emplace_back(base);
	}
	
	uint32_t lastIndex = vertexSize - 1;
	indices.emplace_back(lastIndex);
	
	// add each slice counter clockwise
	const uint32_t lastStackBase = lastIndex - slice;
	for(uint32_t i = 1; i < slice; ++i)
	{
		if(i % 2 == 1)  // line from bottom to top
		{
			for(int32_t j = lastStackBase + i; j > 0; j -= slice)
				indices.emplace_back(j);
			indices.emplace_back(0);
		}
		else  // line from top to bottom
		{
			for(uint32_t j = 1 + i; j < vertexSize; j += slice)
				indices.emplace_back(j);
			indices.emplace_back(lastIndex);
		}
	}
}

void Sphere::pushLinesIndexData(std::vector<uint32_t>& indices)
{
	for(uint32_t j = 0; j < seam; ++j)
	{
		const uint32_t base = 1 + j * slice;
		for(uint32_t i = 0; i < slice; ++i)
		{
			indices.push_back(base + i);
			indices.push_back(base + (i + 1) % slice);
		}
	}
}

void Sphere::pushTriangleStripIndexData(std::vector<uint32_t>& indices, const size_t& vertexSize)
{
	// top stack
	for(uint32_t i = 1; i <= slice; ++i)
	{
		indices.emplace_back(0);  // top index
		indices.emplace_back(i);
	}
	indices.emplace_back(0);  // top index
	indices.emplace_back(1);
	
	// middle part likes a wrapped terrain, but index counts from one.
	for(uint32_t j = 0; j < stack - 2; ++j)
	{
		uint32_t base = 1 + j * slice;
		for(const uint32_t end = base + slice; base < end; ++base)
		{
			indices.emplace_back(base);
			indices.emplace_back(base + slice);
		}
		
		indices.emplace_back(base - slice);
		indices.emplace_back(base);
	}
	
	// bottom stack
	const uint32_t lastIndex = vertexSize - 1;
	for(uint32_t i = lastIndex - slice; i < lastIndex; ++i)
	{
		indices.emplace_back(i);
		indices.emplace_back(lastIndex);  // bottom index
	}
	indices.emplace_back(lastIndex - slice);
	indices.emplace_back(lastIndex);  // bottom index
}

std::vector<uint32_t> Sphere::getVertexIndex(Primitive primitive)
{
	size_t indexSize = getVertexIndexSize(primitive);
	std::vector<uint32_t> indices;
	indices.reserve(indexSize);
	
	const size_t vertexSize = getVertexSize();
	switch(primitive)
	{
	case Primitive::POINTS:
		for(uint32_t i = 0; i < vertexSize; ++i)
			indices.emplace_back(i);
		break;
	
	case Primitive::LINE_STRIP:
		assert(seam == 0);
		pushLineStripIndexData(indices, vertexSize);
		break;
		
	case Primitive::LINES:
		if(seam == 0)
		{
			pushLineStripIndexData(indices, vertexSize);
			indices = lineStripToLines(indices.data(), indices.size());
		}
		else
		{
/*
			for(uint32_t i = 1; i <= slice; ++i)
			{
				indices.emplace_back(0);
				indices.emplace_back(i);
			}
			for(uint32_t j = 0; j < seam; ++j)
			{
				uint32_t base = 1 + j * slice;
				for(uint32_t i = 0; i <= slice; ++i)
				{
					indices.emplace_back(base + i);
					indices.emplace_back((base + (i + 1) % slice);
				}
				
				
			}
*/
		}
		break;
	
	case Primitive::TRIANGLE_STRIP:
		assert(seam == 0);
		pushTriangleStripIndexData(indices, vertexSize);
		break;
		
	case Primitive::TRIANGLES:
		pushTriangleStripIndexData(indices, vertexSize);
		indices = triangleStripToTriangles(indices.data(), indices.size());
		break;
		
	case Primitive::TRIANGLE_FAN:
		// two triangle fan, top and bottom
		for(uint32_t i = 0; i <= slice; ++i)
			indices.push_back(i);
		indices.push_back(1);  // top vertex
		
		indices.push_back(vertexSize - 1);  // last, bottom vertex
		for(uint32_t i = 0; i < slice; ++i)
			indices.push_back(vertexSize - 2 - i);
		indices.push_back(vertexSize - 2);
		break;
	
	case Primitive::QUADRILATERALS:
		for(uint32_t j = 2; j < stack; ++j)
		{
			uint32_t start0 = 1 + (j - 2) * slice;
			uint32_t start1 = start0 + slice;
			for(uint32_t i = 0; i < slice; ++i)
			{
				uint32_t i1 = (i + 1) % slice;
				indices.push_back(start0 + i);
				indices.push_back(start1 + i);
				indices.push_back(start1 + i1);
				indices.push_back(start0 + i1);
			}
		}
		break;
		
	default:
		break;
	}

	assert(indices.size() == indexSize);
	return indices;
}

std::vector<vec2f> Sphere::getTexcoordData(Primitive primitive)
{
/*
	^t
	|  __   __
	| /  \ /  \   <--- S, N polar
	|_\__/_\__/___(1, 1/2)
	|             |
	|             |
	+--------------->s
*/
//	const size_t vertexSize = getVertexSize();
	const size_t indexSize = getVertexIndexSize(primitive);
	std::vector<vec2f> texcoords;
	texcoords.reserve(indexSize);
	if(primitive == Primitive::TRIANGLE_STRIP)
	{
		float r = M_PI / stack;
		float t0 = stack > 2? 0.5:0.0;  // if stack == 2, there are only two polars.
		vec2f southPolar(r, r + t0), northPolar(3*r, r + t0);
		
		std::vector<float> sliceCosineTable(slice + 1), sliceSineTable(slice + 1);
		generateCosineSineTable(sliceCosineTable.data(), sliceSineTable.data(), slice, false/* halfCircle */);
		
		for(uint32_t i = 0; i <= slice; ++i)
		{
			texcoords.push_back(southPolar);
			texcoords.push_back(southPolar + r * vec2f(sliceCosineTable[i], sliceSineTable[i]));
		}
		
		if(stack > 2)
		{
			std::vector<float> sliceTable(slice + 1), stackTable(stack - 2 + 1);
			for(uint32_t i = 0; i <= slice; ++i)
				sliceTable[i] = static_cast<float>(i) / slice;
			for(uint32_t j =  stack - 2; j-- > 0;)
				stackTable[j] = static_cast<float>(j) / ((stack - 2) << 1);
			
			for(uint32_t j = stack - 2; j-- > 0;)
			for(uint32_t i = 0; i <= slice; ++i)
			{
				texcoords.emplace_back(stackTable[j], sliceTable[i]);
				texcoords.emplace_back(stackTable[j - 1], sliceTable[i]);
			}
		}
		
		for(uint32_t i = 0; i < slice; ++i)
		{
			texcoords.push_back(northPolar + r * vec2f(sliceCosineTable[i], sliceSineTable[i]));
			texcoords.push_back(northPolar);
		}
		texcoords.push_back(northPolar + vec2f(r, 0));
	}
	else
	{
		assert(false);  // TODO
		
		
	}
	
	return texcoords;
}
#if 0
std::vector<vec2f> Sphere::getPolarTexcoordData(bool top)
{
	vec2f polar(0.5, 0.5);
	
	std::vector<float> sliceCosineTable(slice + 1), sliceSineTable(slice + 1);
	generateCosineSineTable(sliceCosineTable.data(), sliceSineTable.data(), slice, false/* halfCircle */);
	
}

std::vector<vec2f> Sphere::getBeltTexcoordData()
{
}
#endif
std::vector<vec3f> Sphere::getNormalData()
{
	// sin/cos lookup tables
	std::vector<float> sliceCosineTable(slice + 1), sliceSineTable(slice + 1);
	std::vector<float> stackCosineTable(stack + 1), stackSineTable(stack + 1);
	generateCosineSineTable(sliceCosineTable.data(), sliceSineTable.data(), slice, false/* halfCircle */);
	generateCosineSineTable(stackCosineTable.data(), stackSineTable.data(), stack, true /* halfCircle */);
	
	size_t normalCount = getVertexSize();
	std::vector<vec3f> normals(normalCount);
	
	// each stack, from bottom to top
	int32_t k = 1;
	auto pushRing = [&](uint32_t stack, uint32_t count)
	{
		const float& r = stackSineTable[stack];
		const float& z = stackCosineTable[stack];  // from +1 to -1
		for(uint32_t i = 0; i < count; ++i, ++k)
		{
			float x = r * sliceCosineTable[i];
			float y = r * sliceSineTable[i];
			normals[k] = vec3f(x, y, z);  // z, from +1 to -1, from top to bottom.
		}
	};
	
	normals[0] = vec3f(0, 0, +1);  // top
	
	if(seam == 0)  // no seam
	{
		for(uint32_t j = 1; j < stack; ++j)
			pushRing(j, slice);
	}
	else  // mark with seam
	{
		const uint32_t seamStack0 = 1 + seam;
		const uint32_t seamStack1 = stack - seamStack0;
		for(uint32_t j = 1; j <= seamStack0; ++j)
			pushRing(j, slice);
		
		for(uint32_t j = seamStack0; j <= seamStack1; ++j)
			pushRing(j, slice + 1);  // extra 1 is for seam
		
		for(uint32_t j = seamStack1; j < stack; ++j)
			pushRing(j, slice);
	}
	
	normals[k] = vec3f(0, 0, -1);  // bottom
	return normals;
}
