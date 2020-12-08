#include "geometry/Teapot.h"

#include "geometry/Grid.h"

using namespace pea;

const vec3f Teapot::controlPoints[Teapot::VERTEX_COUNT] =
{
	vec3f{ 1.40000f,  0.00000f,  2.40000f}, vec3f{ 1.40000f, -0.78400f,  2.40000f},
	vec3f{ 0.78400f, -1.40000f,  2.40000f}, vec3f{ 0.00000f, -1.40000f,  2.40000f},
	vec3f{ 1.33750f,  0.00000f,  2.53125f}, vec3f{ 1.33750f, -0.74900f,  2.53125f},
	vec3f{ 0.74900f, -1.33750f,  2.53125f}, vec3f{ 0.00000f, -1.33750f,  2.53125f},
	vec3f{ 1.43750f,  0.00000f,  2.53125f}, vec3f{ 1.43750f, -0.80500f,  2.53125f},
	vec3f{ 0.80500f, -1.43750f,  2.53125f}, vec3f{ 0.00000f, -1.43750f,  2.53125f},
	vec3f{ 1.50000f,  0.00000f,  2.40000f}, vec3f{ 1.50000f, -0.84000f,  2.40000f},
	vec3f{ 0.84000f, -1.50000f,  2.40000f}, vec3f{ 0.00000f, -1.50000f,  2.40000f},
	vec3f{ 1.75000f,  0.00000f,  1.87500f}, vec3f{ 1.75000f, -0.98000f,  1.87500f},
	vec3f{ 0.98000f, -1.75000f,  1.87500f}, vec3f{ 0.00000f, -1.75000f,  1.87500f},
	vec3f{ 2.00000f,  0.00000f,  1.35000f}, vec3f{ 2.00000f, -1.12000f,  1.35000f},
	vec3f{ 1.12000f, -2.00000f,  1.35000f}, vec3f{ 0.00000f, -2.00000f,  1.35000f},
	vec3f{ 2.00000f,  0.00000f,  0.90000f}, vec3f{ 2.00000f, -1.12000f,  0.90000f},
	vec3f{ 1.12000f, -2.00000f,  0.90000f}, vec3f{ 0.00000f, -2.00000f,  0.90000f},
	vec3f{ 2.00000f,  0.00000f,  0.45000f}, vec3f{ 2.00000f, -1.12000f,  0.45000f},
	vec3f{ 1.12000f, -2.00000f,  0.45000f}, vec3f{ 0.00000f, -2.00000f,  0.45000f},
	vec3f{ 1.50000f,  0.00000f,  0.22500f}, vec3f{ 1.50000f, -0.84000f,  0.22500f},
	vec3f{ 0.84000f, -1.50000f,  0.22500f}, vec3f{ 0.00000f, -1.50000f,  0.22500f},
	vec3f{ 1.50000f,  0.00000f,  0.15000f}, vec3f{ 1.50000f, -0.84000f,  0.15000f},
	vec3f{ 0.84000f, -1.50000f,  0.15000f}, vec3f{ 0.00000f, -1.50000f,  0.15000f},
	vec3f{ 0.00000f,  0.00000f,  3.15000f}, vec3f{ 0.00000f, -0.00200f,  3.15000f},
	vec3f{ 0.00200f,  0.00000f,  3.15000f}, vec3f{ 0.80000f,  0.00000f,  3.15000f},
	vec3f{ 0.80000f, -0.45000f,  3.15000f}, vec3f{ 0.45000f, -0.80000f,  3.15000f},
	vec3f{ 0.00000f, -0.80000f,  3.15000f}, vec3f{ 0.00000f,  0.00000f,  2.85000f},
	vec3f{ 0.20000f,  0.00000f,  2.70000f}, vec3f{ 0.20000f, -0.11200f,  2.70000f},
	vec3f{ 0.11200f, -0.20000f,  2.70000f}, vec3f{ 0.00000f, -0.20000f,  2.70000f},
	vec3f{ 0.40000f,  0.00000f,  2.55000f}, vec3f{ 0.40000f, -0.22400f,  2.55000f},
	vec3f{ 0.22400f, -0.40000f,  2.55000f}, vec3f{ 0.00000f, -0.40000f,  2.55000f},
	vec3f{ 1.30000f,  0.00000f,  2.55000f}, vec3f{ 1.30000f, -0.72800f,  2.55000f},
	vec3f{ 0.72800f, -1.30000f,  2.55000f}, vec3f{ 0.00000f, -1.30000f,  2.55000f},
	vec3f{ 1.30000f,  0.00000f,  2.40000f}, vec3f{ 1.30000f, -0.72800f,  2.40000f},
	vec3f{ 0.72800f, -1.30000f,  2.40000f}, vec3f{ 0.00000f, -1.30000f,  2.40000f},
	vec3f{ 0.00000f,  0.00000f,  0.00000f}, vec3f{ 0.00000f, -1.42500f,  0.00000f},
	vec3f{ 0.79800f, -1.42500f,  0.00000f}, vec3f{ 1.42500f, -0.79800f,  0.00000f},
	vec3f{ 1.42500f,  0.00000f,  0.00000f}, vec3f{ 0.00000f, -1.50000f,  0.07500f},
	vec3f{ 0.84000f, -1.50000f,  0.07500f}, vec3f{ 1.50000f, -0.84000f,  0.07500f},
	vec3f{ 1.50000f,  0.00000f,  0.07500f}, vec3f{-1.60000f,  0.00000f,  2.02500f},
	vec3f{-1.60000f, -0.30000f,  2.02500f}, vec3f{-1.50000f, -0.30000f,  2.25000f},
	vec3f{-1.50000f,  0.00000f,  2.25000f}, vec3f{-2.30000f,  0.00000f,  2.02500f},
	vec3f{-2.30000f, -0.30000f,  2.02500f}, vec3f{-2.50000f, -0.30000f,  2.25000f},
	vec3f{-2.50000f,  0.00000f,  2.25000f}, vec3f{-2.70000f,  0.00000f,  2.02500f},
	vec3f{-2.70000f, -0.30000f,  2.02500f}, vec3f{-3.00000f, -0.30000f,  2.25000f},
	vec3f{-3.00000f,  0.00000f,  2.25000f}, vec3f{-2.70000f,  0.00000f,  1.80000f},
	vec3f{-2.70000f, -0.30000f,  1.80000f}, vec3f{-3.00000f, -0.30000f,  1.80000f},
	vec3f{-3.00000f,  0.00000f,  1.80000f}, vec3f{-2.70000f,  0.00000f,  1.57500f},
	vec3f{-2.70000f, -0.30000f,  1.57500f}, vec3f{-3.00000f, -0.30000f,  1.35000f},
	vec3f{-3.00000f,  0.00000f,  1.35000f}, vec3f{-2.50000f,  0.00000f,  1.12500f},
	vec3f{-2.50000f, -0.30000f,  1.12500f}, vec3f{-2.65000f, -0.30000f,  0.93750f},
	vec3f{-2.65000f,  0.00000f,  0.93750f}, vec3f{-2.00000f,  0.00000f,  0.90000f},
	vec3f{-2.00000f, -0.30000f,  0.90000f}, vec3f{-1.90000f, -0.30000f,  0.60000f},
	vec3f{-1.90000f,  0.00000f,  0.60000f}, vec3f{ 1.70000f,  0.00000f,  1.42500f},
	vec3f{ 1.70000f, -0.66000f,  1.42500f}, vec3f{ 1.70000f, -0.66000f,  0.60000f},
	vec3f{ 1.70000f,  0.00000f,  0.60000f}, vec3f{ 2.60000f,  0.00000f,  1.42500f},
	vec3f{ 2.60000f, -0.66000f,  1.42500f}, vec3f{ 3.10000f, -0.66000f,  0.82500f},
	vec3f{ 3.10000f,  0.00000f,  0.82500f}, vec3f{ 2.30000f,  0.00000f,  2.10000f},
	vec3f{ 2.30000f, -0.25000f,  2.10000f}, vec3f{ 2.40000f, -0.25000f,  2.02500f},
	vec3f{ 2.40000f,  0.00000f,  2.02500f}, vec3f{ 2.70000f,  0.00000f,  2.40000f},
	vec3f{ 2.70000f, -0.25000f,  2.40000f}, vec3f{ 3.30000f, -0.25000f,  2.40000f},
	vec3f{ 3.30000f,  0.00000f,  2.40000f}, vec3f{ 2.80000f,  0.00000f,  2.47500f},
	vec3f{ 2.80000f, -0.25000f,  2.47500f}, vec3f{ 3.52500f, -0.25000f,  2.49375f},
	vec3f{ 3.52500f,  0.00000f,  2.49375f}, vec3f{ 2.90000f,  0.00000f,  2.47500f},
	vec3f{ 2.90000f, -0.15000f,  2.47500f}, vec3f{ 3.45000f, -0.15000f,  2.51250f},
	vec3f{ 3.45000f,  0.00000f,  2.51250f}, vec3f{ 2.80000f,  0.00000f,  2.40000f},
	vec3f{ 2.80000f, -0.15000f,  2.40000f}, vec3f{ 3.20000f, -0.15000f,  2.40000f},
	vec3f{ 3.20000f,  0.00000f,  2.40000f}
};

constexpr uint16_t ROTATION_PATCH_END = 6;
const uint16_t Teapot::patchIndices[Teapot::PATCH_COUNT][16] =
{
	{  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15},  // rim
	{ 12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27},  // body
	{ 24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39},
	{ 40,  41,  42,  40,  43,  44,  45,  46,  47,  47,  47,  47,  48,  49,  50,  51},  // lid
	{ 48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63},
	{ 64,  64,  64,  64,  65,  66,  67,  68,  69,  70,  71,  72,  39,  38,  37,  36},  // bottom
	// Rim, body, lid, and bottom data must be rotated along all four quadrants;
	// handle and spout data is flipped across the x-y plane (negate z values) only.
	{ 73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88},  // handle
	{ 85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100},
	{101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116},  // spout
	{113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128},
};

Teapot::Teapot(int32_t subdivision):
		patch(subdivision)
{
}

int32_t Teapot::getSubdivision() const
{
	return patch.getSubdivision();
}

/**
 * rotate patch around Z axis to other 3 quadrants
 * @param[inout] array the end of patch data, start of patch data to generate.
 * @param[in] data length of one patch.
 */
static void rotatePatch(vec3f* array, int32_t n)
{
	for(int32_t i = 0; i < n; ++i)
	{
		const vec3f& p = array[i - n];
		// Rotating point (x, y, z) around Z axis by angle theta, Z coordinate is unmodified.
		// On complex plane, it can be seen as (x,y) multiplies (cos(theta), sin(theta)).
		array[        i] = vec3f(-p.y, +p.x, p.z);  //  90 degrees: (x + y*i) *  i = -y + x * i
		array[n     + i] = vec3f(-p.x, -p.y, p.z);  // 180 degrees: (x + y*i) * -1 = -x - y * i
		array[n * 2 + i] = vec3f(+p.y, -p.x, p.z);  // 270 degrees: (x + y*i) * -i =  y - x * i
	}
}

// mirror patch across X0Z plane
static void mirrorPatch(vec3f* array, int32_t subdivision)
{
	for(int32_t v = 0; v < subdivision; ++v)
	{
		int32_t src = (v - subdivision) * subdivision;
		int32_t dst = v * subdivision;
		for(int32_t u = 0; u < subdivision; ++u)
		{
			// reversing row order to keep winding correct, and negating y value to perform the flip
			const vec3f& point = array[src + subdivision - u];
			array[dst + u] = vec3f(point.x, -point.y, point.z);
		}
	}
}

// There are 10 patches for a teapot, the first 6 patches needs to be rotated around y axis to 
// other 3 quadrants, and the last 4 patches needs to be mirrored with respect to X0Y Plane.
constexpr uint16_t patchCount = ROTATION_PATCH_END * 4 + (Teapot::PATCH_COUNT - ROTATION_PATCH_END) * 2;
size_t Teapot::getVertexSize() const
{
	size_t size = patch.getSubdivision() + 1;
	return patchCount * size * size;
}

std::vector<vec3f> Teapot::getVertexData() const
{
	const int32_t size = patch.getSubdivision() + 1;
	const size_t vertexCountPerPatch = size * size;
	std::vector<vec3f> vertices(patchCount * vertexCountPerPatch);
	size_t count = 0;
	for(uint16_t p = 0; p < PATCH_COUNT; ++p)
	{
		const uint16_t* index = patchIndices[p];
		for(int32_t v = 0; v < size; ++v)
			for(int32_t u = 0; u < size; ++u)
				vertices[count++] = patch.getPosition(controlPoints, index, u, v);
		
		if(p < ROTATION_PATCH_END)
		{
			rotatePatch(&vertices[count], vertexCountPerPatch);
			count += 3 * vertexCountPerPatch;
		}
		else // if(p < 10)
		{
			mirrorPatch(&vertices[count], size);
			count += vertexCountPerPatch;
		}
	}
	
	assert(count == getVertexSize());
	return vertices;
}

// TODO: remove duplicate code. getVertexData() and getNormalData() only differs in one call.
std::vector<vec3f> Teapot::getNormalData() const
{
	const int32_t size = patch.getSubdivision() + 1;
	const size_t vertexCountPerPatch = size * size;
	std::vector<vec3f> normals(patchCount * vertexCountPerPatch);
	
	size_t count = 0;
	for(uint16_t p = 0; p < PATCH_COUNT; ++p)
	{
		const uint16_t* index = patchIndices[p];
		for(int32_t v = 0; v < size; ++v)
			for(int32_t u = 0; u < size; ++u)
				normals[count++] = patch.getNormal(controlPoints, index, u, v);
		
		if(p < ROTATION_PATCH_END)
		{
			rotatePatch(&normals[count], vertexCountPerPatch);
			count += 3 * vertexCountPerPatch;
		}
		else // if(p < 10)
		{
			mirrorPatch(&normals[count], size);
			count += vertexCountPerPatch;
		}
	}
	
	assert(count == getVertexSize());
	return normals;
}

std::vector<uint32_t> Teapot::getIndexData(Primitive primitive) const
{
	std::vector<uint32_t> indices;
	if(primitive == Primitive::POINTS)
	{
		size_t size = getVertexSize();
		indices.resize(size);
		for(size_t i = 0; i < size; ++i)
			indices[i] = i;
	}
	else if(primitive == Primitive::LINES ||
			primitive == Primitive::TRIANGLES || primitive == Primitive::TRIANGLE_STRIP ||
			primitive == Primitive::QUADRILATERALS || primitive == Primitive::QUADRILATERAL_STRIP)
	{
		const bool strip = primitive == Primitive::TRIANGLE_STRIP ||
				primitive == Primitive::QUADRILATERAL_STRIP;
		
		const int32_t subdivision = patch.getSubdivision();
		std::vector<uint32_t> IndicesPerPatch = Grid::getIndexData(subdivision, subdivision, primitive);
		const size_t sizePerPatch = IndicesPerPatch.size();
		size_t size = patchCount * sizePerPatch;
		if(strip)  // triangle strip needs extra two to connect each patch.
			size += (patchCount - 1) << 1;
		indices.reserve(size);
		const size_t vertexPerPatch = (subdivision + 1) * (subdivision + 1);
		for(uint16_t p = 0; p < patchCount; ++p)
		{
			uint32_t offset = p * vertexPerPatch;
			for(const uint32_t& index: IndicesPerPatch)
				indices.push_back(offset + index);
			
			if(strip && p + 1 < patchCount)
			{
				indices.push_back(offset + IndicesPerPatch[IndicesPerPatch.size() - 1]);
				indices.push_back(offset + vertexPerPatch + IndicesPerPatch[0]);
			}
		}
		assert(indices.size() == size);
	}
	
	return indices;
}
