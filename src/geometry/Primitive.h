#ifndef PEA_GEOMETRY_PRIMITIVE_H_
#define PEA_GEOMETRY_PRIMITIVE_H_

#include <cstdint>
#include <vector>


//#include "util/compiler.h"

namespace pea {

enum class Primitive: std::uint32_t
{
	// vim /usr/include/GL/gl.h +165
	POINTS = 0,
	LINES,
	LINE_LOOP,
	LINE_STRIP,
	TRIANGLES,
	TRIANGLE_STRIP,
	TRIANGLE_FAN,
	QUADRILATERALS,
	QUADRILATERAL_STRIP,
	POLYGON,

//	PRIMITIVE_TYPE_COUNT = POLYGON
};

// primitive restart index
// uint8_t  0xFF
// uint16_t 0xFFFF
// uint32_t 0xFFFFFFFF
/*
enum class VertexFlag: std::uint32_t
{
	USE_PRIMITIVE_RESTART = 1,  // GL_PRIMITIVE_RESTART
	WIREFRAME             = 1 << 1,  // GL_LINE_STRIP
	PACKED_TEXCOORD       = 1 << 2,  // texcoords are packed in one texture
	CIRCLE_UNWRAP         = 1 << 3,  // mark seam, generate two vertex for 0 and 2 * pi.
};
*/

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
