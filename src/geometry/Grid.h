#ifndef PEA_GEOMETRY_GRID_H_
#define PEA_GEOMETRY_GRID_H_

#include <vector>

#include "math/vec2.h"
#include "math/vec3.h"
#include "geometry/Primitive.h"


namespace pea {

/**
 * @class Terrain use OpenGL coordinate
 *
 *
 * @a doc/strip.jpg
 * The grid will be built with a single triangle strip in a snake-like motion.
 * To prevent this extra triangle, we have to use what are called <b>degenerate
 * triangle</b>s, or triangles with no volume. A triangle with no volume will not
 * be rendered. To create the degenerate triangle, we simply repeat the last
 * vertex of each row. However, since each adjacent triangle has the opposite
 * winding, we need to repeat the vertex once more or else the triangle would
 * be considered back-facing.
 *
 * top down pattern
 *
 *     vertex           line                  index
 * 0---1---2---3    0---2---4---6		0 4 1 5 2 6 3 7
 * |   |   |   |    |  /|  /|  /|
 * |   |   |   |    | / | / | / |
 * |   |   |   |    |/  |/  |/  7
 * 4---5---6---7    1---3---5---8		7 4 (degenerated triangle index)
 * |   |   |   |    9---C---E---G
 * |   |   |   |    A  /|  /|  /|
 * |   |   |   |    | / | / | / |
 * |   |   |   |    |/  |/  |/  |
 * 8---9---A---B    B---D---F---H		4 8 5 9 6 A 7 B
 *
 * vertex 0 1 2 3 || 4 5 6 7 || 8 9 A B
 * index  0 4 1 5 2 6 3 7 || 7 4 || 4 8 5 9 6 A 7 B
 *
 *
 * bottom up pattern
 *
 *     vertex           line                  index
 * 8---9---A---B    9---C---E---G		8 4 9 5 A 6 B 7
 * |   |   |   |    A  /|  /|  /|
 * |   |   |   |    | / | / | / |
 * |   |   |   |    B/  |/  |/  |
 * |   |   |   |    9---D---F---H
 * 4---5---6---7    0---2---4---6		3 8 (degenerated triangle index)
 * |   |   |   |    |  /|  /|  /|
 * |   |   |   |    | / | / | / |
 * |   |   |   |    |/  |/  |/  8
 * 0---1---2---3    1---3---5---7		4 0 5 1 6 2 7 3
 *
 * vertex 0 1 2 3 || 4 5 6 7 || 8 9 A B
 * index  4 0 5 1 6 2 7 3 || 3 8 || 8 4 9 5 A 6 B 7
 */
class Grid
{
private:
/*
	vec2u size;
	std::vector<vec3f> vertices;
	std::vector<uint32_t> indices;
*/
public:
	Grid() = default;
	~Grid() = default;
	
	static constexpr size_t getVertexSize(uint32_t stepsX, uint32_t stepsY);
	
	/**
	 * @param[in] start Starting number of the sequence.
	 * @param[in] stop Generate numbers up to, this number is included too.
	 * @param[in] steps At least 1 step from start to stop.
	 * @return from bottom left (start, start, 0) to top right (stop, stop, 0), of size 
	 *         (steps + 1) x (steps + 1) 
	 */
	static std::vector<vec3f> getVertexData(const float& start, const float& stop, const uint32_t& steps);
	
	static std::vector<vec3f> getVertexData(const vec2f& start, const vec2f& stop, const vec2u& steps);

	/**
	 * @param[in] stepsX steps on X axis, at least 1.
	 * @param[in] stepsY steps on Y axis, at least 1.
	 * @param[in] step Adjacent vertices' (X or Y) distance.
	 */
	static std::vector<vec3f> getVertexData(const uint32_t& stepsX, const uint32_t& stepsY, const float& step);
	
	static std::vector<vec2f> getTexcoordData(const float& width, const float& height, const uint32_t& stepsX, const uint32_t& stepsY);
	
	static constexpr size_t getIndexSize(uint32_t stepsX, uint32_t stepsY, Primitive primitive);
	
	/**
	 * create triangle strip indices (from left to right, bottom to top).
	 * @param[in] stepsX subdivision on X axis, at least 1.
	 * @param[in] stepsY subdivision on Y axis, at least 1.
	 * @param[in] strip  true for triangle strip; otherwise triangles.
	 */
	static std::vector<uint32_t> getIndexData(uint32_t stepsX, uint32_t stepsY, Primitive primitive);

};

constexpr size_t Grid::getVertexSize(uint32_t stepsX, uint32_t stepsY)
{
	return (stepsX + 1U) * (stepsY + 1U);
}

constexpr size_t Grid::getIndexSize(uint32_t stepsX, uint32_t stepsY, Primitive primitive)
{
	switch(primitive)
	{
	case Primitive::POINTS:
		return getVertexSize(stepsX, stepsY);
	
	case Primitive::LINES:
		// stepsX * (stepsY + 1) + (stepX + 1) * stepsY  lines
		return (stepsX * stepsY * 2 + stepsX + stepsY) << 1U;
	
	case Primitive::TRIANGLES:
		return stepsX * stepsY * 6;
	
	case Primitive::TRIANGLE_STRIP:
	case Primitive::QUADRILATERAL_STRIP:
		if(stepsX > 0 && stepsY > 0)
			return (stepsX + 2) * 2 * stepsY - 2U;
		else
			return 0;
	
	case Primitive::QUADRILATERALS:
		return stepsX * stepsY * 4;
	
	default:
		return 0;
	}
}

}  // namespace pea
#endif  // PEA_GEOMETRY_GRID_H_
