#include "geometry/BoundingBox.h"

#include "util/utility.h"

using namespace pea;

constexpr float EXTEND = 1.0F;

BoundingBox::BoundingBox():
		min(+EXTEND),
		max(-EXTEND)
{
}

BoundingBox::BoundingBox(const vec3f& min, const vec3f& max):
		min(min),
		max(max)
{
}

void BoundingBox::reset()
{
	min = vec3(+EXTEND);
	max = vec3(-EXTEND);
}

bool BoundingBox::isEmpty() const
{
	return min.x >= max.x || min.y >= max.y || min.z >= max.z;
}

void BoundingBox::repair()
{
	if(min.x > max.x)  std::swap(min.x, max.x);
	if(min.y > max.y)  std::swap(min.y, max.y);
	if(min.z > max.z)  std::swap(min.z, max.z);
}

void BoundingBox::add(const vec3f& point)
{
	auto expand = [](float& low, float& high, const float& value)
	{
		if(value < low)
			low = value;
		else if(value > high)
			high = value;
	};
		
	if(isEmpty())
		min = max = point;
	else
	{
		expand(min.x, max.x, point.x);
		expand(min.y, max.y, point.y);
		expand(min.z, max.z, point.z);
	}
}

bool BoundingBox::contain(const vec3f& point) const
{
	return min.x <= point.x && point.x <= max.x &&
			min.y <= point.y && point.y <= max.y &&
			min.z <= point.z && point.z <= max.z;
}

bool BoundingBox::overlap(const BoundingBox &other) const
{
	return (*this & other).isEmpty();
}

vec3f BoundingBox::center() const
{
	return (min + max) * 0.5F;
}

float BoundingBox::volume() const
{
	vec3f e = max - min;
	float v = e.x * e.y * e.z;

	assert(v >= 0);
	return v;
}

namespace pea {

BoundingBox operator &(const BoundingBox& lhs, const BoundingBox& rhs)
{
	if(lhs.isEmpty())
		return lhs;
	if(rhs.isEmpty())
		return rhs;
	
	float min_x = std::max(lhs.min.x, rhs.min.x);
	float min_y = std::max(lhs.min.y, rhs.min.y);
	float min_z = std::max(lhs.min.z, rhs.min.z);
	
	float max_x = std::min(lhs.max.x, rhs.max.x);
	float max_y = std::min(lhs.max.y, rhs.max.y);
	float max_z = std::min(lhs.max.z, rhs.max.z);
	
	return BoundingBox(vec3(min_x, min_y, min_z), vec3(max_x, max_y, max_z));
}

BoundingBox operator |(const BoundingBox& lhs, const BoundingBox& rhs)
{
	if(lhs.isEmpty())
		return rhs;
	if(rhs.isEmpty())
		return lhs;
	
	float min_x = std::min(lhs.min.x, rhs.min.x);
	float min_y = std::min(lhs.min.y, rhs.min.y);
	float min_z = std::min(lhs.min.z, rhs.min.z);
	
	float max_x = std::max(lhs.max.x, rhs.max.x);
	float max_y = std::max(lhs.max.y, rhs.max.y);
	float max_z = std::max(lhs.max.z, rhs.max.z);
	
	return BoundingBox(vec3(min_x, min_y, min_z), vec3(max_x, max_y, max_z));
}

}  // namespace pea
