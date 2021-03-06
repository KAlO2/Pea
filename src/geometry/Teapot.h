#ifndef PEA_GEOMETRY_TEAPOT_H_
#define PEA_GEOMETRY_TEAPOT_H_

#include "geometry/BezierPatch.h"
#include "opengl/Primitive.h"

namespace pea {

/**
 * data derived from an archive uploaded by Juhana Kouhia, downloaded from:
 * ftp://ftp.funet.fi/pub/sci/graphics/packages/objects/teasetorig.gz
 * Utah teapot has 306 vertices and the indices that define 32 Bezier patches.
 * @see http://nautil.us/blog/the-most-important-object-in-computer-graphics-history-is-this-teapot
 * @see https://www.sjbaker.org/wiki/index.php?title=The_History_of_The_Teapot
 */
class Teapot
{
public:
	static constexpr uint16_t PATCH_COUNT  = 10;//32;
	static constexpr uint16_t VERTEX_COUNT = 129;//269;//306;
	
	static const vec3f controlPoints[VERTEX_COUNT];
	static const uint16_t patchIndices[PATCH_COUNT][16];

private:
	BezierPatch patch;
	
public:
	explicit Teapot(int32_t subdivision);
	
	int32_t getSubdivision() const;
	
	size_t getVertexSize() const;
	
	std::vector<vec3f> getVertexData() const;
	
	std::vector<vec3f> getNormalData() const;
	
	std::vector<uint32_t> getIndexData(Primitive primitive) const;
};

}  // namespace pea
#endif  // PEA_GEOMETRY_TEAPOT_H_
