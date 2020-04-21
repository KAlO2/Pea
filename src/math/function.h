#ifndef PEA_MATH_FUNCTION_H_
#define PEA_MATH_FUNCTION_H_

#include "math/vec3.h"

namespace pea {

/**
 * @param[out] cost Cosine table, length is n + 1.
 * @param[out] sint Sine table, length is n + 1.
 * @param[in] n     slice of the (half) circle.
 */
void generateCosineSineTable(float* cost, float* sint, uint32_t n, bool halfCircle = false);

template <typename T>
T log2i(T x)
{
	static_assert(std::is_integral<T>::value, "expect an integer type");
	assert(x > 0);
#if 0
	// x86 has bsr instruction, while arm has clz instruction
	return sizeof(T) * CHAR_BITS - __builtin_clz(x) - 1;
	
	uint32_t y;
	asm("\tbsr %1, %0\n": "=r"(y): "r" (x));
	return y;
#else
	T bit = 0;
	while(x >>= 1)
		++bit;
	return bit;
#endif
}

/**
 * use std::pow for float exponent, use pea::pow for integer exponent.
 */
template <typename T>
T pow(T base, uint32_t exp)
{
	T result = 1;
	while(exp > 0)
	{
		if(exp & 1)
			result *= base;
		exp >>= 1;
		base *= base;
	}
	
	return result;
}

constexpr int32_t factorial(int32_t n)
{
	// $ calc
	// ; 2^31 = 2147483648
	// ;12! = 479001600, 13! = 6227020800
	assert(0 <= n && n < 12);
	if(n <= 1)
		return 1;
	
	return n * factorial(n - 1);
}

/**
 * C(n, k) = n! / (n - k)! * k!
 */
int32_t binomialCoefficient(int32_t n, int32_t k);

/**
 * b_k_n(t) = C(n, k) * t^i * (1 - t)^(n - k)
 */
float bernstein(int32_t n, int32_t k, float t);

//float bernsteinDerivative(int32_t n, int32_t k, float t);
/*
 *
 */
vec3f bezier(const vec3f& p0, const vec3f& p1, const vec3f& p2, const vec3f& p3, float t);

}  // namespace pea
#endif  // PEA_MATH_FUNCTION_H_
