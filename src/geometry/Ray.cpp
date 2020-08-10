#include "geometry/Ray.h"

using namespace pea;

static inline void assertUnitVector(const vec3f& vector)
{
	assert(std::abs(vector.length2() - 1) < 1E-6);
}

Ray Ray::from(const vec3f& point0, const vec3f& point1)
{
	assert(point0 != point1);
	vec3f direction = (point1 - point0).normalize();
	return Ray(point0, direction);
}

Ray::Ray(const vec3f& origin, const vec3f& direction):
		origin(origin),
		direction(direction)
{
	assertUnitVector(direction);
}

void Ray::setDirection(const vec3f& direction)
{
	assertUnitVector(direction);
	this->direction = direction;
}

vec3f Ray::at(float t) const
{
	return origin + direction * t;
}

/*
	       _-^| point
	    _-^   |
	 _-^      |
	O-------------
	         projection
*/
float Ray::distance(const vec3f& point) const
{
	vec3f vector = point - origin;
	vec3f projection = dot(vector, direction) * direction;
	return (vector - projection).length();
}

