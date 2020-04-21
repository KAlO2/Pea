#ifndef PEA_GEOMETRY_CUBE_H_
#define PEA_GEOMETRY_CUBE_H_

#include <vector>

#include "geometry/Primitive.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace pea {

class Cube
{
public:
	enum Face: uint8_t
	{
		LEFT  = 0,
		RIGHT = 1,
		BACK  = 2,
		FRONT = 3,
		BOTTOM= 4,
		TOP   = 5,
		
		FACE_COUNT = 6,
	};
	
public:
	Cube() = delete;
	~Cube() = delete;

	static constexpr size_t getVertexSize() { return 8; }
	
	/**
	 * @param[in] size size of the cube. width, depth, height.
	 * @return cube's vertex data. Its center is (0, 0, 0).
	 */
	static std::vector<vec3f> getVertexData(const vec3f& size);
	
	/**
	 * Get vertex data from a bouding box 
	 */
	static std::vector<vec3f> getVertexData(const vec3f& min, const vec3f& max);
	
	static size_t getIndexSize(Primitive primitive);
	
	/**
	 * Returns cube's index data, with respect to cube's vertex data.
	 */
	static std::vector<uint8_t> getVertexIndex(Primitive primitive);
	
	static std::vector<vec2f> getTexcoordData();
	
	static std::vector<uint8_t> getTexcoordIndex(Primitive primitive);
	
	static std::vector<vec2f> getPackedTexcoordData(Primitive primitive);
	
	/**
	 * Returns vertex normal data, not face normal data. data are normalized.
	 */
	static std::vector<vec3f> getNormalData();
	
	static std::vector<uint8_t> getNormalIndex(Primitive primitive);
};

}  // namespace pea
#endif  // PEA_GEOMETRY_CUBE_H_
