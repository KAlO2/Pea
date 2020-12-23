#ifndef PEA_GEOMETRY_BOUNDING_BOX_H_
#define PEA_GEOMETRY_BOUNDING_BOX_H_

#include "math/vec3.h"

namespace pea {

/**
 * @class Axis-Aligned Bounding Box
 */
class BoundingBox
{
private:
	vec3f min, max;

public:
	/**
	 * construct an empty box.
	 */
	BoundingBox();
	
	/**
	 * construct an box with lower and upper bounds. Note that if @p min is not element-wise less
	 * than @p max, a #repare() operation is needed aftewards.
	 */
	BoundingBox(const vec3f& min, const vec3f& max);
	
	~BoundingBox() = default;

	const vec3f& getLowerBound() const;
	const vec3f& getUpperBound() const;
	
	/**
	 * Resets the bounding box.
	 *
	 * @param value New box to set this one to.
	 */
	void reset();

	bool isEmpty() const;

	/**
	 * @return box's size of each dimensions.
	 */
	vec3f getSize() const;

	/**
	 * Repairs the box. Swap minimum and maximum value if necessary.
	 */
	void repair();

	/**
	 * Adds a point to the bounding box. The box will grow bigger if vector is outside of the box.
	 *
	 * @param[in] point The point to be added
	 */
	void add(const vec3f& point);
	
	/*
	 * Expand the box uniformly for 3 axes.
	 * @param[in] amount The amount to grow, can be negative.
	 */
	void expand(float amount);
	
	/**
	 * Expand the box to some extend. You use positive value to grow, negative value to shrink. when
	 * box shrinks too much, it becomes a center point.
	 * @param[in] amount The amount to grow, can be negative.
	 */
	void expand(const vec3f& amount);
	
	/**
	 * Adds another bounding box. The box will grow bigger if vector is outside of the box.
	 *
	 * @param box the BoundingBox box to be added
	 */
	void add(const BoundingBox& box);

	bool contain(const vec3f& point) const;
	
	bool overlap(const BoundingBox& other) const;

	float calculateVolume() const;

	vec3f getCenter() const;
	
	/**
	 * Computes the intersection of two bounding boxes.
	 */
	friend BoundingBox operator &(const BoundingBox& lhs, const BoundingBox& rhs);
	
	/**
	 * Computes the union of two bounding boxes.
	 */
	friend BoundingBox operator |(const BoundingBox& lhs, const BoundingBox& rhs);
};

inline const vec3f& BoundingBox::getLowerBound() const { return min; }
inline const vec3f& BoundingBox::getUpperBound() const { return max; }

}  // namespace pea
#endif  // PEA_GEOMETRY_BOUNDING_BOX_H_
