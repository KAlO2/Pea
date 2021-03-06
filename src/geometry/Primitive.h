#ifndef PEA_GEOMETRY_PRIMITIVE_H_
#define PEA_GEOMETRY_PRIMITIVE_H_

#include <cstdint>
#include <vector>


namespace pea {

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
