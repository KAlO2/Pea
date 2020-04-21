#ifndef PEA_MATH_SCALAR_H_
#define PEA_MATH_SCALAR_H_

#include <cmath>
#include <limits>
#include <type_traits>
#include <cassert>
#include <cstdint>

namespace pea {

using std::int8_t;
using std::int16_t;
using std::int32_t;
using std::int64_t;

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

/**
 *  A 16-bit floating-point number has a 1-bit sign (S), a 5-bit
 *  exponent (E), and a 10-bit mantissa (M).  The value of a 16-bit
 *  floating-point number is determined by the following:
 *
 *      (-1)^S * 0.0,                        if E == 0 and M == 0,
 *      (-1)^S * 2^-14 * (M / 2^10),         if E == 0 and M != 0,
 *      (-1)^S * 2^(E-15) * (1 + M/2^10),    if 0 < E < 31,
 *      (-1)^S * INF,                        if E == 31 and M == 0, or
 *      NaN,                                 if E == 31 and M != 0,
 *
 *  where
 *
 *      S = floor((N mod 65536) / 32768),
 *      E = floor((N mod 32768) / 1024), and
 *      M = N mod 1024.
 */
union float16_t
{
	uint16_t value;
	struct
	{
		uint16_t fractal: 10;
		uint16_t exponent: 5;
		uint16_t sign: 1;
	} IEEE;
	// NAN() : mantissa(0x3FF), exponent(0x1F), sign(0x1) {}
};
using half = float16_t;

union float32_t
{
	float value;
	struct
	{
		uint32_t fractal: 23;
		uint32_t exponent: 8;
		uint32_t sign: 1;
	} IEEE;
	// NAN() : mantissa(0x7FFFFF), exponent(0xFF), sign(0x1) {}
};

union float64_t
{
	double value;
	struct
	{
		uint64_t fractal: 52;
		uint64_t exponent: 11;
		uint64_t sign: 1;
	} IEEE;
	// NAN() :mantissa(0xFFFFFFFFFFFFFL), exponent(0x7FFL), sign(0x1L) {}
};
/*
union float80_t
{
	long double value;
	struct
	{
		uint80_t fractal: 65;
		uint80_t exponent: 14;
		uint80_t sign: 1;
	} IEEE;
	// NAN() :mantissa(0xFFFFFFFFFFFFFL), exponent(0x7FFL), sign(0x1L) {}
};
*/

#ifdef USE_DOUBLE_PRECISION
	using real = double;
#else
	using real = float;
#endif // USE_DOUBLE_PRECISION

// template variable is brought in C++14
// @see http://en.cppreference.com/w/cpp/language/variable_template
template <typename T>
class scalar
{
public:
// @see http://en.cppreference.com/w/cpp/preprocessor/replace for __cplusplus macro.

	static constexpr T tolerance = 1.0E-5;
	static constexpr T pi = static_cast<T>(M_PI);
	static constexpr T e = static_cast<T>(M_E);
};

template <typename T>
inline bool isZero(const T& value)
{
//	static_assert(!std::is_floating_point<T>::value, "limited to scalar floating-point type only");
	static_assert(std::is_integral<T>::value, "limited to integral type only");
	return value == 0;
}

/*
	template specialization for real types. We limit difference precisions with difference tolerances.
	Intuitively, when you fully specialize something, it doesn't depend on a template parameter any
	more, so unless you make the specialization inline, you need to put it in a .cpp file instead of
	a .h or you end up violating the one definition rule (http://en.wikipedia.org/wiki/One_Definition_Rule).
	Note that when you partially specialize templates, the partial specializations do still depend on
	one or more template parameters, so they still go in a .h file.
*/
template <>
inline bool isZero<float>(const float& value)
{
	return std::abs(value) <= std::sqrt(std::numeric_limits<float>::epsilon());
}

template <>
inline bool isZero<double>(const double& value)
{
	// std::cbrt(x) is not equivalent to std::pow(x, 1.0/3) because std::pow cannot raise a
	// negative base to a fractional exponent.
	return std::abs(value) <= std::cbrt(std::numeric_limits<double>::epsilon());
}

template <>
inline bool isZero<long double>(const long double& value)
{
	return std::abs(value) <= std::pow(std::numeric_limits<long double>::epsilon(), 1.0/4);
}

/**
 * @brief Compare two floating point values for equality, with a permissible
 * amount of error. Oftentimes you only care if floats are close enough and
 * this function lets you make that determination. For vector and matrix with
 * float type, operator == is your choice.
 *
 * (Because of rounding errors inherent in floating point arithmetic, direct
 * comparison of floats is often inadvisable.
 * http://en.wikipedia.org/wiki/Floating_point#Accuracy_problems )
 *
 * @param a The first floating value
 * @param b The second floating value
 * @return Whether the two floating values are within epsilon of each other
 */
template <typename T>
inline bool fuzzyEqual(const T& a, const T& b)
{
	// http://floating-point-gui.de/errors/comparison/
	static_assert(std::is_floating_point<T>::value, "limited to scalar floating-point type only");
	if(a == b)  // shortcut, handles infinities
		return true;
//	else if(a == T(0) || b == T(0) || isZero<T>(a - b))
	return isZero<T>(a - b);
}

template <>
inline bool fuzzyEqual<int32_t>(const int& a, const int& b)
{
	return a == b;
}

template <>
inline bool fuzzyEqual<uint32_t>(const uint32_t& a, const uint32_t& b)
{
	return a == b;
}

template <>
inline bool fuzzyEqual<std::size_t>(const std::size_t& a, const std::size_t& b)
{
	return a == b;
}

// C++20 got bool has_single_bit(T x); function in header <bit>
template <typename T>
inline bool isPowerOfTwo(T n)
{
	static_assert(std::is_integral<T>::value, "limited to integral type only");
	return (n > 0) && ((n & (n - 1)) == 0);
}

/**
 * @brief Convert degrees to radians
 *
 * @param degrees The angle in degrees
 * @return The angle in radians
 */
template <typename T>
inline T deg2rad(const T& degrees)
{
	static_assert(std::is_floating_point<T>::value, "limited to floating-point type only");
	return degrees / (180 / static_cast<T>(M_PI));
}

/**
 * @brief Convert radians to degrees
 *
 * @param radians The angle in radians
 * @return The angle in degrees
 */
template <typename T>
inline T rad2deg(const T& radians)
{
	static_assert(std::is_floating_point<T>::value, "limited to floating-point type only");
	return radians * (180 / static_cast<T>(M_PI));
}

/*
 * @see C++11 feature about user literal, http://en.cppreference.com/w/cpp/language/user_literal
 * note that float or double type are not allowed on literal operators
 */
constexpr long double operator "" _deg(long double degree)
{
	return degree * static_cast<long double>(M_PI) / 180;
}

constexpr long double operator "" _deg(unsigned long long int degree)
{
	return degree * static_cast<long double>(M_PI) / 180;
}

/**
 * @brief Whether the two line segments on one axis overlaps
 *
 * @param l1min The lower bound of the first line segment
 * @param l1max The upper bound of the first line segment
 * @param l2min The lower bound of the second line segment
 * @param l2max The upper bound of the second line segment
 * @return true if the two interval [l1min, l1max] and
 *   [l2min, l2max] overlaps, otherwise false.
 */
template <typename T>
inline bool overlap(const T& l1min, const T& l1max,
		const T& l2min, const T& l2max)
{
	assert(l1min <= l1max && l2min <= l2max);
//	const T len = (l1max - l1min) + (l2max - l2min);
//	return (l2max - l1min < len) && (l1max - l2min < len);
	return std::max(l1min, l2min) < std::min(l1max, l2max);
}

/**
 * @brief Clamps a value to a specified range [min, max]
 *
 * @param value The double in question
 * @param min The minimum of the range
 * @param max The maximum of the range
 * @return The clamped value
 */
template <typename T>
inline const T clamp(const T& value, const T& min, const T& max)
{
	assert(min <= max && "invalid clamp range");
#if 0
	return std::min<T>(std::max<T>(value, min), max);
#else
	if(value < min)
		return min;
	if(value > max)
		return max;
	return value;
#endif
}

/**
 * @brief Linearly interpolates between two values. Works for any classes that
 * define addition, subtraction, and multiplication (by a float) operators.
 *
 * http://en.wikipedia.org/wiki/Lerp_(computing)
 *
 * @param start The starting value
 * @param end The ending value
 * @param amount The amount to lerp (from 0.0 to 1.0)
 * @return The interpolated value
 */
template <typename T>
inline T lerp(const T& start, const T& end, const T& amount)
{
	static_assert(std::is_floating_point<T>::value);
	return start + (end - start) * amount;
}

/**
 * Smoothly step between two values. Works for any classes that lerp would work
 * for (and is essentially a drop-in replacement). Often looks visually better
 * than a simple linear interpolation as it gives ease-in and ease-out.
 *
 * http://www.fundza.com/rman_shaders/smoothstep/index.html
 *
 * @param start The starting value
 * @param end The ending value
 * @param amount The amount to interpolate (from 0.0 to 1.0)
 * @return The interpolated value
 */
template <typename T>
inline T smoothStep(const T& start, const T& end, const T& amount)
{
	float a = amount * amount * (3 - 2 * amount);
	return lerp<T>(start, end, a);
}

/**
 * Normal distribution, also known as Gaussian distribution.
 *	            1             (x - μ)^2
 *	f(x) = ------------ exp(- ---------)
 *	       sqrt(2*pi)*σ         2*σ^2
 *
 * P(μ-σ<X≤μ+σ) = 68.3%, P(μ-2σ<X≤μ+2σ) = 95.4%, P(μ-3σ<X≤μ+3σ) = 99.7%.
 */
template <typename T>
inline T gaussian(const T& x, const T& miu = T(0), const T& sigma = T(1))
{
	T t = (x - miu) / sigma;
	constexpr T f = 2.50662827463100050242;  // std::sqrt(2 * M_PI)
	return std::exp<T>(t * t / static_cast<T>(-2)) / f / sigma;
}

}  // namespace pea
#endif  // PEA_MATH_SCALAR_H_
