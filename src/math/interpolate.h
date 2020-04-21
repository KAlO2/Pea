#ifndef PEA_MATH_INTERPOLATE_H_
#define PEA_MATH_INTERPOLATE_H_


// linear Bezier curve
template<typename T>
T bezier(const T& p0, const T& p1, float t)
{
	return (1-t)*p0 + t*p1;
}

// Quadratic Bezier curve
template<typename T>
T bezier(const T& p0, const T& p1, const T& p2, float t)
{
	T q0 = bezier(p0, p1, t);
	T q1 = bezier(p1, p2, t);
	return (q0, q1, t);
}

// Cubic Bezier curve
template<typename T>
T bezier(const T& p0, const T& p1, const T& p2, const T& p3, float t)
{
#if 1
	T q0 = bezier(p0, p1, t);
	T q1 = bezier(p1, p2, t);
	T q2 = bezier(p2, p3, t);
	q0 = bezier(q0, q1, t);
	q1 = bezier(q1, q2, t);
	return bezier(q0, q1, t);
#else
	float tmp0 = t*t;
	float tmp1 = t - tmp0;  // t*(1-t)
	float tmp2 = (1-t)*(1-t);
	return tmp2*((1-t)*p0 + 3*t*p1) + tmp0*(3*(1-t)*p2 + t*p3);
#endif
}



#endif  // PEA_MATH_INTERPOLATE_H_
