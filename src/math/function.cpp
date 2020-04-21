#include "math/function.h"

#include <cmath>

namespace pea {

void generateCosineSineTable(float* cost, float* sint, uint32_t n, bool halfCircle/* = false*/)
{
	assert(cost != nullptr && sint != nullptr && n > 0);

	const double angle =(1 + !halfCircle) * M_PI / n;
	cost[0] = 1;
	sint[0] = 0; 
	for(uint32_t i = 1; i < n; ++i)
	{
		double t = i * angle;
		cost[i] = std::cos(t);
		sint[i] = std::sin(t);
	}
	
	if(halfCircle)
	{
		cost[n] =-1;  // cos(pi) = -1
		sint[n] = 0;  // sin(pi) = 0
	}
	else
	{
		cost[n] = cost[0];  // cos(2*pi + t) = cos(t)
		sint[n] = sint[0];  // sin(2*pi + t) = sin(t)
	}

//	sint[n - i] = s;  cost[n - i] =-c;  // sin(pi - x) = sin(x), cos(pi - x) = -cos(x)
}

int32_t binomialCoefficient(int32_t n, int32_t k)
{
	assert(0 <= k && k <= n);
	static int32_t array[12] = {0};
	
	if(array[0])
		return array[n] / (array[n - k] * array[k]);  // c(n, k) = n! / ((n-k)! * k!)
	else
	{
		array[0] = 1;
		for(int32_t i = 2; i < n; ++i)
			array[i] = i * array[i - 1];
		return array[k];
	}
}

float bernstein(int32_t n, int32_t k, float t)
{
	float c = binomialCoefficient(n, k);
	if(t == 0 || t == 1)
		return c;
	return c * std::pow(t, k) * std::pow(1 - t, n - k);
}
/*
float bernsteinDerivative(int32_t n, int32_t k, float t)
{
	float c = binomialCoefficient(n, k);
	if(t == 0 || t == 1)
		return c;
#if 0
	float d0 = k == 0? 1.0F: std::pow(t, k - 1);
	float d1 = K == n? 1.0F: std::pow(1 - t, n - k - 1);
	return c * (k * d0 * d1*(1- t) - k * t * d1 * (n - k));
#else
	float f;
	if(k == 0)
		return -n * std::pow(1 - t, n - 1);
	else if(k == n)
		return n * std::pow(t, n - 1);
	else
		return k * std::pow(t, k - 1) * std::pow(1 - t, n - k - 1) - (n - k) * std::pow(t, k) * std::pow(1 - t, n - k - 1);
	return c * f;
#endif
}
*/

// The De Casteljau algorithm is numerically more stable way (compared to using the parametric form 
// directly) of evaluating the position of a point on the curve for any given t (it only requires a 
// series of linear interpolation).
// (1 - t) * p0 + t * p1;  ==> 6 -, 18 +, 36 *
// p0 + t * (p1 - p0);     ==> 18 -, 18+, 18 *
vec3f bezier(const vec3f& p0, const vec3f& p1, const vec3f& p2, const vec3f& p3, float t)
{
#if 0
	// compute first tree points along main segments p0-p1, p1-p2 and p2-p3
	vec3f p01 = lerp(p0, p1, t);
	vec3f p12 = lerp(p1, p2, t);
	vec3f p23 = lerp(p2, p3, t);
	
	// compute two points along segments p0p1-p1p2 and p1p2-p2p3
	vec3f p012 = lerp(p01, p12, t);
	vec3f p123 = lerp(p12, p23, t);

	// finally compute p
	vec3f p0123 = lerp(p012, p123, t);
	return p0123;
#else
//	1 - ,9+, 21 *
	float r  = 1 - t;
	float tt = t * t;
	float rr = r * r;
	float k0 = r * rr;
	float k1 = rr * t;  // 3 * rr * t;
	float k2 = r * tt;  // 3 * r * tt;
	float k3 = tt * t;
	return k0 * p0 + 3 * (k1 * p1 + k2 * p2) + k3 * p3;
#endif
}

}  // namespace pea
