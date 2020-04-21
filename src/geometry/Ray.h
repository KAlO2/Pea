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
	vec3f position;
	vec3f direction;  ///< normalized vector

public:
	Ray(const vec3f& position, const vec3f& direction);

	void setPosition(const vec3f& position);
	const vec3f& getPosition() const;

	void setDirection(const vec3f& direction);
	const vec3f& getDirection() const;

	/**
	 * shorthand for @code setPosition(getPosition() + offset) @endcode
	 */
	Ray& operator +=(const vec3f& offset);
	
	/**
	 * shorthand for @code setPosition(getPosition() - offset) @endcode
	 */
	Ray& operator -=(const vec3f& offset);

	/**
	 * @param[in] t coordinate along this ray, can be negative.
	 * @return point on the ray.
	 */
	vec3f at(float t) const;
};

inline void         Ray::setPosition(const vec3f& position) { this->position = position; }
inline const vec3f& Ray::getPosition() const                { return position;           }

inline const vec3f& Ray::getDirection() const { return direction; }

inline Ray& Ray::operator +=(const vec3f& offset) { position += offset; return *this; }
inline Ray& Ray::operator -=(const vec3f& offset) { position -= offset; return *this; }

}  // namespace pea
#endif  // PEA_GEOMETRY_RAY_H_
