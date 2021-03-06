#ifndef PEA_GEOMETRY_CYLINDER_H_
#define PEA_GEOMETRY_CYLINDER_H_

#include <vector>

#include "math/vec2.h"
#include "math/vec3.h"
#include "opengl/Primitive.h"

namespace pea {

enum class CapFillType: uint8_t
{
	NONE,
	TRIANGLE_FAN,
	POLYGON,
};

/**
 * @class Cylinder implements a cylinder shape primitive.
 * A sphere and a cylinder are homeomorphic, a sphere and a torus are not homeomorphic.
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
	static int32_t slice;
	static int32_t stack;
	static CapFillType cap;
	
public:
	/**
	 * @param[in] slice number of fans around cross section.
	 */
	static void    setSlice(int32_t slice);
	static int32_t getSlice();
	
	/**
	 * @param[in] stack number of stack along cross section.
	 */
	static void    setStack(int32_t stack);
	static int32_t getStack();
	
	static void        setCapFillType(CapFillType cap);
	static CapFillType getCapFillType();
	
	static size_t getVertexSize();
	
	/**
	 * vertex data are arranged from bottom to top. bottom center comes first, and top center reside
	 * last.
	 */
	std::vector<vec3f> getVertexData() const;
	
	static size_t getIndexSize(Primitive primitive);
	
	static std::vector<uint32_t> getVertexIndex(Primitive primitive);
	
	static std::vector<vec3f> getNormalData();
	
	static std::vector<uint32_t> getNormalIndex(Primitive primitive);
	
	std::vector<vec2f> getTexcoordData(Primitive primitive) const;
};

inline const float& Cylinder::getHeight() const { return height; }
inline const float& Cylinder::getRadius() const { return radius; }

inline int32_t Cylinder::getSlice() { return slice; }
inline int32_t Cylinder::getStack() { return stack; }

inline void Cylinder::setCapFillType(CapFillType cap) { Cylinder::cap = cap; }
inline CapFillType Cylinder::getCapFillType() { return cap; }

}  // namespace pea
#endif  // PEA_GEOMETRY_CYLINDER_H_
