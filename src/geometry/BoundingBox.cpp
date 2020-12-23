#include "geometry/BoundingBox.h"

#include "util/utility.h"

using namespace pea;

constexpr float NaN = std::numeric_limits<float>::quiet_NaN();

BoundingBox::BoundingBox():
		min(NaN),
		max(NaN)
{
}

BoundingBox::BoundingBox(const vec3f& min, const vec3f& max):
		min(min),
		max(max)
{
	auto isNaN = [](const vec3f& v) {return std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z);};
	assert(!isNaN(min) && !isNaN(max));
}

void BoundingBox::reset()
{
	min = vec3(NaN);
	max = vec3(NaN);
}

bool BoundingBox::isEmpty() const
{
	return std::isnan(min.x);
}

vec3f BoundingBox::getSize() const
{
	return max - min;
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
		assert(low <= high);
		if(value < low)
			low = value;
		else if(value > high)
			high = value;
	};
	
	if(isEmpty()) [[unlikely]]
		min = max = point;
	else
	{
		expand(min.x, max.x, point.x);
		expand(min.y, max.y, point.y);
		expand(min.z, max.z, point.z);
	}
}

void BoundingBox::expand(float amount)
{
	expand(vec3f(amount));
}

void BoundingBox::expand(const vec3f& amount)
{
	auto expand = [](float& low, float& high, const float& delta)
	{
		assert(low <= high && std::isfinite(delta));
		low -= delta;
		high += delta;
		
		if(low > high)  // shrink to a point
			low = high = (low + high) * 0.5F;
	};
	
	expand(min.x, max.x, amount.x);
	expand(min.y, max.y, amount.y);
	expand(min.z, max.z, amount.z);
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

vec3f BoundingBox::getCenter() const
{
	return (min + max) * 0.5F;
}

float BoundingBox::calculateVolume() const
{
	vec3f e = max - min;
	assert(e.x >= 0 && e.y >= 0 && e.z >= 0);
	return e.x * e.y * e.z;
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
