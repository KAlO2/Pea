#ifndef PEA_GRAPHICS_RECT_H_
#define PEA_GRAPHICS_RECT_H_

namespace pea {

template <typename T>
class Rect
{
public:
	T left, right, top, bottom;

public:
	Rect();
	Rect(T left, T right, T top, T bottom);
	Rect(const Rect& other) = default;
	
	Rect& operator = (const Rect& other);
	
	/**
	 * Returns true if the rectangle is empty (left >= right or top >= bottom)
	 */
	bool isEmpty() const;
	
	/**
	 * Set the rectangle to (0,0,0,0)
	 */
	void setEmpty();
	
	/**
	 * Swap top/bottom or left/right if there are flipped (i.e. left > right
	 * and/or top > bottom). This can be called if
	 * the edges are computed separately, and may have crossed over each other.
	 * If the edges are already correct (i.e. left <= right and top <= bottom)
	 * then nothing is done.
	 */
	void sort();
	
	/**
	 * @return the rectangle's width. This does not check for a valid rectangle
	 * (i.e. left <= right) so the result may be negative.
	 */
	T getWidth() const;
	
	/**
	 * @return the rectangle's height. This does not check for a valid rectangle
	 * (i.e. top <= bottom) so the result may be negative.
	 */
	T getHeight() const;
	
	/**
	 * Returns true if (x,y) is inside the rectangle. The left and top are
	 * considered to be inside, while the right and bottom are not. This means
	 * that for a x,y to be contained: left <= x < right and top <= y < bottom.
	 * An empty rectangle never contains any point.
	 *
	 * @param x The X coordinate of the point being tested for containment
	 * @param y The Y coordinate of the point being tested for containment
	 * @return true iff (x,y) are contained by the rectangle, where containment
	 *              means left <= x < right and top <= y < bottom
	 */
	bool contains(T x, T y) const;
	
	/**
	 * Returns true iff the 4 specified sides of a rectangle are inside or equal
	 * to this rectangle. i.e. is this rectangle a superset of the specified
	 * rectangle. An empty rectangle never contains another rectangle.
	 *
	 * @param[in] rect The rectangle being tested for containment
	 * @return true iff the the 4 specified sides of a rectangle are inside or
	 *              equal to this rectangle
	 */
	bool contains(const Rect<T>& rect) const;
	
	/**
	 * Update this Rect to enclose itself and the [x,y] coordinate. There is no
	 * check to see that this rectangle is non-empty.
	 *
	 * @param[in] x The x coordinate of the point to add to the rectangle
	 * @param[in] y The y coordinate of the point to add to the rectangle
	 */
	void union_(T x, T y);
	
	/**
	 * Update this Rect to enclose itself and the specified rectangle. If the
	 * specified rectangle is empty, nothing is done. If this rectangle is empty
	 * it is set to the specified rectangle.
	 *
	 * @param[in] rect The rectangle being unioned with this rectangle
	 */
	void union_(const Rect<T>& rect);
	
	/**
	 * Offset the rectangle by adding dx to its left and right coordinates, and
	 * adding dy to its top and bottom coordinates.
	 *
	 * @param[in] dx The amount to add to the rectangle's left and right coordinates
	 * @param[in] dy The amount to add to the rectangle's top and bottom coordinates
	 */
	void offset(T dx, T dy);
	
	void inset(T x);
	
	
	
	void scale(float factor);
};

}  // namespace pea

#include "graphics/Rect.inl"
#endif  // PEA_GRAPHICS_RECT_H_
