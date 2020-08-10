#ifndef PEA_GEOMETRY_RAY_H_
#define PEA_GEOMETRY_RAY_H_

#include "math/vec3.h"

namespace pea {

/**
 * A ray is a line with an origin and direction in space.
 */
class Ray
{
private:
	vec3f origin;
	vec3f direction;  ///< normalized vector

public:
	/**
	 * construct a ray with two points.
	 */
	static Ray from(const vec3f& point0, const vec3f& point1);
	
	/**
	 * construct a ray with origin and direction.
	 */
	Ray(const vec3f& origin, const vec3f& direction);

	void setOrigin(const vec3f& origin);
	const vec3f& getOrigin() const;

	void setDirection(const vec3f& direction);
	const vec3f& getDirection() const;

	/**
	 * shorthand for @code setorigin(getorigin() + offset) @endcode
	 */
	Ray& operator +=(const vec3f& offset);
	
	/**
	 * shorthand for @code setorigin(getorigin() - offset) @endcode
	 */
	Ray& operator -=(const vec3f& offset);

	/**
	 * @param[in] t coordinate along this ray, can be negative.
	 * @return point on the ray.
	 */
	vec3f at(float t) const;
	
	/**
	 * distance between point and line
	 */
	float distance(const vec3f& point) const;
};

inline void         Ray::setOrigin(const vec3f& origin) { this->origin = origin; }
inline const vec3f& Ray::getOrigin() const              { return origin;         }

inline const vec3f& Ray::getDirection() const { return direction; }

inline Ray& Ray::operator +=(const vec3f& offset) { origin += offset; return *this; }
inline Ray& Ray::operator -=(const vec3f& offset) { origin -= offset; return *this; }

}  // namespace pea
#endif  // PEA_GEOMETRY_RAY_H_
