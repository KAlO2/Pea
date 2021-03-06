#ifndef PEA_OPENGL_PRIMITIVE_H_
#define PEA_OPENGL_PRIMITIVE_H_

namespace pea {
/**
 * https://www.khronos.org/opengl/wiki/Primitive
 * POINTS can be drawn with
 *     glDrawArrays(GL_POINTS, 0, Geometry::getIndexSize());
 * which you don't have to supply indice data.
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

}  // namespace pea
#endif  // PEA_OPENGL_PRIMITIVE_H_
