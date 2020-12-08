#include "geometry/BezierCurve.h"

using namespace pea;

BezierCurve::BezierCurve(int32_t subdivision):
		_bernstein0(subdivision + 1),
		_bernstein1(subdivision + 1)
{
	assert(subdivision > 0);
	
	//  The basis functions and their derivatives for the n=3 case are:
	// B_{0, 3} = (1 - t)^3;          B_{0, 3}' = -3 * (1 - t)^2;
	// B_{1, 3} = 3 * t * (1 - t)^2;  B_{0, 3}' = 3 * (1 - t)^2 - 6 * u * (1 - u);
	// B_{2, 3} = 3 * t^2 * (1 - t);  B_{0, 3}' = 6 * t * (1 - t) - 3 * t^2;
	// B_{3, 3} = t^3;                B_{0, 3}' = 3 * t^2;
	const float delta = 1.0F / subdivision;
	for(int32_t n = 0; n <= subdivision; ++n)
	{
		float t = n * delta;
		float s = 1 - t;
		float tt = t * t, ttt = tt * t;
		float ss = s * s, sss = ss * s;
		_bernstein0[n] = vec4f(sss, 3 * t * ss, 3 * tt * s, ttt);
		_bernstein1[n] = vec4f(-3 * ss, 3 * ss - 6 * t * s, 6 * t * s - 3 * tt, 3 * tt);
	}
}

int32_t BezierCurve::getSubdivision() const
{
	return _bernstein0.size() - 1;
}
