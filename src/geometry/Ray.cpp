#include "geometry/Ray.h"

using namespace pea;

static inline void assertUnitVector(const vec3f& vector)
{
	assert(std::abs(vector.length2() - 1) < 1E-6);
}

Ray::Ray(const vec3f& position, const vec3f& direction):
		position(position),
		direction(direction)
{
//	assertUnitVector(direction);
}

void Ray::setDirection(const vec3f& direction)
{
//	assertUnitVector(direction);
	this->direction = direction;
}

vec3f Ray::at(float t) const
{
	return position + direction * t;
}
