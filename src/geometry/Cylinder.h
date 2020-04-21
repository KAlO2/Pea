#ifndef PEA_GEOMETRY_CYLINDER_H_
#define PEA_GEOMETRY_CYLINDER_H_

#include <vector>

#include "geometry/Primitive.h"
#include "math/vec2.h"
#include "math/vec3.h"

namespace pea {

enum class CapFillType: uint8_t
{
	NONE,
	TRIANGLE_FAN,
	NGON,
};

/**
 * @class Cylinder implements a cylinder shape primitive.
 * With its generatrix parallelling to Z axis, center being the origin.
 */
class Cylinder
{
private:
	float radius, height;
public:
	Cylinder(float radius, float height);
	~Cylinder() = default;

	const float& getHeight() const;
	const float& getRadius() const;
	
	vec3f ineria(float mass);
	
private:
	static uint32_t slice;
	static CapFillType cap;
	
public:
	static void     setSlice(uint32_t slice);
	static uint32_t getSlice();
	
	static void        setCapFillType(CapFillType cap);
	static CapFillType getCapFillType();
	
	static size_t getVertexSize();
	
	std::vector<vec3f> getVertexData() const;
	
	static size_t getIndexSize(Primitive primitive);
	
	static std::vector<uint32_t> getVertexIndex(Primitive primitive);
	
	static std::vector<vec3f> getNormalData();
	
	static std::vector<uint32_t> getNormalIndex(Primitive primitive);
	
	std::vector<vec2f> getTexcoordData(Primitive primitive) const;
};

inline const float& Cylinder::getHeight() const { return height; }
inline const float& Cylinder::getRadius() const { return radius; }

inline uint32_t Cylinder::getSlice() { return slice; }

inline void Cylinder::setCapFillType(CapFillType cap) { Cylinder::cap = cap; }
inline CapFillType Cylinder::getCapFillType() { return cap; }

}  // namespace pea
#endif  // PEA_GEOMETRY_CYLINDER_H_
