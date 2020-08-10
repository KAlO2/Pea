#ifndef PEA_GEOMETRY_PRIMITIVE_H_
#define PEA_GEOMETRY_PRIMITIVE_H_

#include <cstdint>
#include <vector>


namespace pea {

/**
 * https://www.khronos.org/opengl/wiki/Primitive
 */
enum class Primitive: std::uint32_t
{
	// vim /usr/include/GL/gl.h +165
	POINTS         = 0x00,
	LINES          = 0x01,
	LINE_LOOP      = 0x02,
	LINE_STRIP     = 0x03,
	TRIANGLES      = 0x04,
	TRIANGLE_STRIP = 0x05,
	TRIANGLE_FAN   = 0x06,
	QUADRILATERALS = 0x07,
	QUADRILATERAL_STRIP = 0x08,
	POLYGON        = 0x09,

	// Primitive Adjacency
	LINES_ADJACENCY          = 0x0A,
	LINE_STRIP_ADJACENCY     = 0x0B,
	TRIANGLES_ADJACENCY      = 0x0C,
	TRIANGLE_STRIP_ADJACENCY = 0x0D,

	// Tessellation
	PATCHES        = 0x0E,
//	PRIMITIVE_TYPE_COUNT = PATCHES,
};

// primitive restart index
// uint8_t  0xFF
// uint16_t 0xFFFF
// uint32_t 0xFFFFFFFF

std::vector<uint32_t> lineStripToLines(const uint32_t* strip, size_t length);

std::vector<uint32_t> lineLoopToLines(const uint32_t* strip, size_t length);

/**
 * turn triangle strip indices to stringles indices.
 * @param[in] strip A triangle strip.
 * @param[in] length Strip element length. It must be at least 3 for anything to be drawn.
 */
std::vector<uint32_t> triangleStripToTriangles(const uint32_t* strip, size_t length);

std::vector<uint32_t> triangleFanToTriangles(const uint32_t* strip, size_t length);

std::vector<uint32_t> quadrilateralsToTriangles(const uint32_t* quad, size_t length);

std::vector<uint32_t> polygonsToTriangles(const uint32_t* polygon, const uint32_t* vertexSizes, size_t count);

}  // namespace pea
#endif  // PEA_GEOMETRY_PRIMITIVE_H_
