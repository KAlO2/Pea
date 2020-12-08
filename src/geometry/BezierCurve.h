#ifndef PEA_GEOMETRY_BEZIER_CURVE_H_
#define PEA_GEOMETRY_BEZIER_CURVE_H_

#include <vector>

#include "math/vec4.h"

namespace pea {

/**
 * The Bézier Curve is a type of spline curve with two end points and two control points.
 * Best used to calculate the position of an object traveling along such a curve at a particular 
 * time (t).
 * 
 * A Bézier Curve can have any number of control points and is defined by the formula:
 *         N           N!     k      1-k
 * B(u) = sum P(k) --------- u *(1-u)
 *        k=0      k!*(N-k)!
 * Where N is the number of the last point, P(k) is point #k, and u is the distance along the curve,
 * where u=0 is the beginning point P(0) and u=1 is the end point P(N).
 *
 * x0 = end point 1, x1 = control point 1, x2 = control point 2, x3 = end point 3.
 * X(t) = A*t^3 + B*t^2 + C*t + x0
 * where C = 3*(x1 - x0), B = 3*(x2 - x1) - C, A = x3 - x0 - C - B;
 * or in a single line:
 * x(t) = (x3 - 3*x2 + 3*x1 - x0)*t^3 + 3*(x2 - 2*x1 + x0)*t^2 + 3*(x1 - x0)*t + x0
 * If derived, the equations can then be used to find the relative velocity of the object.
 * v(t) = 3*A*t^2 + 2*B*t + C
 * and again for the acceleration.
 * a(t) = 6*A*t + 2*B
 */
class BezierCurve
{
private:
	std::vector<vec4f> _bernstein0;  // Bernstain polynomial
	std::vector<vec4f> _bernstein1;  // Bernstain polynomial's 1st derivative
	
public:
	/**
	 * @param[in] subdivision segment of the curve.
	 */
	explicit BezierCurve(int32_t subdivision);
	
	/**
	 * @return subdivision value, at least 1.
	 */
	int32_t getSubdivision() const;
	
	/**
	 * @param[in] k range [0, subdivision].
	 * @return coefficient of k-th point.
	 */
	const vec4f& bernstein0(int32_t k) const;
	
	/**
	 * @param[in] k range [0, subdivision].
	 * @return coefficient of k-th point's derivative.
	 */
	const vec4f& bernstein1(int32_t k) const;
};

inline const vec4f& BezierCurve::bernstein0(int32_t k) const { return _bernstein0[k]; }
inline const vec4f& BezierCurve::bernstein1(int32_t k) const { return _bernstein1[k]; }

}  // namespace pea
#endif  // PEA_GEOMETRY_BEZIER_CURVE_H_
