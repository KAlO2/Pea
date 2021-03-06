#ifndef PEA_GEOMETRY_TETRAHEDRON_H_
#define PEA_GEOMETRY_TETRAHEDRON_H_

#include <vector>

#include "math/vec2.h"
#include "math/vec3.h"
#include "opengl/Primitive.h"

namespace pea {

/**
 * Regular tetrahedron, can UV unwrap to a plane perfectly.
 * http://mathworld.wolfram.com/RegularTetrahedron.html
 */
class Tetrahedron
{
public:
	static float getInradius(float edgeLength);
	
	/**
	 * @param[in] edgeLength edge length
	 */
	static float getCircumradius(float edgeLength);
	
	static constexpr size_t getVertexSize() { return 4; }
	static constexpr size_t getIndexSize()  { return 4 * 3; }
	
	/**
	 * @param[in] edgeLength edge length of the tetrahedron.
	 * @return 4 vertex positions
	 */
	static std::vector<vec3f> getVertexData(float edgeLength);
	
	static std::vector<uint8_t> getVertexIndex(Primitive primitive);
	
	static std::vector<vec3f> getNormalData();
	
	static std::vector<vec2f> getTexcoordData();

};

}  // namespace pea
#endif  // PEA_GEOMETRY_TETRAHEDRON_H_
