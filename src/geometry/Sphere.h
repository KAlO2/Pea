#ifndef PEA_GEOMETRY_SPHERE_H_
#define PEA_GEOMETRY_SPHERE_H_

#include <vector>

#include "math/vec2.h"
#include "math/vec3.h"
#include "opengl/Primitive.h"

namespace pea {

/**
 * @brief a primitive shape, mostly used for bounds checking.
 * (x-a)^2 + (y-b)^2 + (z-c)^2 = r^2, where (a, b, c) is the center, r the radius.
 *
 * You can get ellipsoid (x/a)^2 + (y/b)^2 + (z/c)^2 = 1 by scaling sphere,
 * where (a, b, c) denotes radius on each axes.
 */
class Sphere
{
private:
	vec3f center;
	float radius;
	
public:
	Sphere();
	Sphere(const vec3f& center, float radius);
	Sphere(const Sphere& other) = default;
	~Sphere() = default;

	Sphere& operator =(const Sphere& rhs);

	const vec3f& getPosition() const;
	const float& getRadius() const;
	
	void setPosition(const vec3f& position);
	void setRadius(float radius);
	
	/**
	 * Merges another Sphere into the current sphere
	 */
	void merge(const Sphere& other);

	bool intersect(const Sphere& other);
	
	static constexpr float volume(float radius);
	
	static bool intersect(const Sphere& lhs, const Sphere& rhs);
	
	/**
	 * compose pitch and yaw angle into forward unit vector.
	 * @param[in] pitch Latitude, interval [-pi/2, pi/2], pitch is zero in horizontal distance.
	 * @param[in] yaw   Longitude interval [-pi, pi), yaw is zero in right direction.
	 */
	static vec3f composeOrientation(const float& pitch, const float& yaw);
	
	/**
	 * @param[in] forward A unit vector point forward.
	 * @return (pitch, yaw) Euler angle pair. or sphere coordinates (latitude, longitude).
	 */
	static std::pair<float, float> decomposeOrientation(const vec3f& forward);
	
	/**
	 * Constrain pitch in interval [-pi/2, pi)
	 * @param[in] pitch Pitch may be out of interval [-pi/2, pi/2)
	 * @param[in] yaw
	 * @return true if changed pitch and yaw.
	 */
	static bool wrap(float& pitch, float& yaw);
	
private:
	static uint32_t slice;  ///< The number of subdivisions around the Z axis (similar to lines of longitude).
	static uint32_t stack;  ///< The number of subdivisions along the Z axis (similar to lines of latitude).
	static uint32_t seam;   ///< mark seam on a stack, 0 means no seam.
	
private:
	static void pushLineStripIndexData(std::vector<uint32_t>& indices, const size_t& vertexSize);
	static void pushLinesIndexData(std::vector<uint32_t>& indices);
	
	static void pushTriangleStripIndexData(std::vector<uint32_t>& indices, const size_t& vertexSize);
	
public:
	static void setSlice(uint32_t slice);
	static void setStack(uint32_t stack);
	
	/**
	 * To mark seam on a stack, used in UV unwrap.
	 * @param[in] stack a stack number, from top to bottom. 0 means no seam.
	 */
	static void setSeam(uint32_t seam);
	
	static uint32_t getSlice();
	static uint32_t getStack();
	static uint32_t getSeam();
	
	static size_t getVertexSize();
	static size_t getIndexSize(Primitive primitive);
	
	std::vector<vec3f> getVertexData() const;
	
	static size_t getVertexIndexSize(Primitive primitive);
	
	/**
	 * @param[in] primitive LINE_STRIP, TRIANGLES or TRIANGLE_STRIP
	 *  
	 * Note that you can shrink index type uint32_t to uint16_t or uint8_t if size is not big.
	 */
	static std::vector<uint32_t> getVertexIndex(Primitive primitive);
	
	/**
	 * generate texture coordinates with respect to solid index data.
	 */
	static std::vector<vec2f> getTexcoordData(Primitive primitive);
	
//	static std::vector<vec2f> getPolarTexcoordData(uint32_t polarStack, bool top);
	
//	static std::vector<vec2f> getBeltTexcoordData(uint32_t polarStack);
	
	/**
	 * Generate per vertex normal
	 */
	static std::vector<vec3f> getNormalData();
};

constexpr float Sphere::volume(float radius)
{
	constexpr double s = M_PI * 4 / 3;
	return s * (radius * radius * radius);
}

inline const vec3f& Sphere::getPosition() const { return center; }
inline const float& Sphere::getRadius() const   { return radius; }

inline void Sphere::setPosition(const vec3f& position) { this->center = position; }
inline void Sphere::setRadius(float radius) { assert(radius > 0); this->radius = radius; }

inline uint32_t Sphere::getSlice() { return slice; }
inline uint32_t Sphere::getStack() { return stack; }
inline uint32_t Sphere::getSeam()  { return seam; }

}  // namespace pea
#endif  // PEA_GEOMETRY_SPHERE_H_
