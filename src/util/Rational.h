#ifndef PEA_UTIL_RATIONAL_H_
#define PEA_UTIL_RATIONAL_H_

#include <sstream>

namespace pea {

// forward declarations for friend functions
template <typename T>
class Rational;

template <typename T>
bool operator >(const Rational<T>& lhs, const Rational<T>& rhs);

template <typename T>
bool operator ==(const Rational<T>& lhs, const Rational<T>& rhs);

template <typename T>
bool operator <(const Rational<T>& lhs, const Rational<T>& rhs);


template <typename T>
class Rational
{
private:
	static_assert(std::is_signed<T>::value && std::is_integral<T>::value, "signed integral type required.");
	T num, den;  // represents rational num / den.
	
public:
	// Damn! NAN is a macro defined in <cmath> header, we have to change the name to NaN.
	static const Rational<T> NaN;  // = Rational<T>(0, 0);
	static const Rational<T> POSITIVE_ZERO;      // = Rational<T>(0, +1);
	static const Rational<T> NEGATIVE_ZERO;      // = Rational<T>(0, -1);
	static const Rational<T> POSITIVE_INFINITY;  // = Rational<T>(+1, 0);
	static const Rational<T> NEGATIVE_INFINITY;  // = Rational<T>(-1, 0);
	
private:
	/**
	 * Computes the greatest common divisor of the integers a and b.
	 *
	 * @return If both a and b are zero, returns zero. Otherwise, returns the greatest common 
	 *         divisor of |a| and |b|.
	 *
	 * @see std::gcd since C++17.
	 */
	static constexpr T gcd(T a, T b);
	
	/**
	 * When we say a rational numerator / denominator is normalized, we mean gcd(num, den) == 1, and
	 * den >= 0.
	 */
	constexpr void normalize();
	
	constexpr Rational(T numerator, T denominator, bool normalize_);
public:
	using value_type = T;
	
	constexpr Rational(T n);  ///< construct rational n / 1
	constexpr Rational(T numerator, T denominator);  ///< construct rational numerator / denominator
	
	constexpr T getNumerator()   const { return num; }
	constexpr T getDenominator() const { return den; }
	
	constexpr bool isNaN()    const { return num == 0 && den == 0; }
	constexpr bool isZero()   const { return num == 0 && den != 0; }
	constexpr bool isFinite() const { return             den != 0; }
	
	constexpr bool isPositive() const         { return num > 0 && den  > 0; }
	constexpr bool isNegative() const         { return num < 0 && den  > 0; }
	constexpr bool isPositiveInfinity() const { return num > 0 && den == 0; }
	constexpr bool isNegativeInfinity() const { return num < 0 && den == 0; }
	
	constexpr Rational<T> reciprocal() const;
	
	// unary operators
	constexpr Rational<T> operator+ () const { return Rational<T>(+num, den, false); }
	constexpr Rational<T> operator- () const { return Rational<T>(-num, den, false); }
	
	// arithmetic
	Rational<T>& operator+= (const Rational<T>& other);
	Rational<T>& operator-= (const Rational<T>& other);
	Rational<T>& operator*= (const Rational<T>& other);
	Rational<T>& operator/= (const Rational<T>& other);
	
	friend Rational<T> operator+ (const Rational<T>& lhs, const Rational<T>& rhs) { Rational<T> r = lhs; r += rhs; return r; }
	friend Rational<T> operator- (const Rational<T>& lhs, const Rational<T>& rhs) { Rational<T> r = lhs; r -= rhs; return r; }
	friend Rational<T> operator* (const Rational<T>& lhs, const Rational<T>& rhs) { Rational<T> r = lhs; r *= rhs; return r; }
	friend Rational<T> operator/ (const Rational<T>& lhs, const Rational<T>& rhs) { Rational<T> r = lhs; r /= rhs; return r; }
	
	// comparison
	friend bool operator<  <>(const Rational<T>& lhs, const Rational<T>& rhs);
	friend bool operator>  <>(const Rational<T>& lhs, const Rational<T>& rhs);
	friend bool operator== <>(const Rational<T>& lhs, const Rational<T>& rhs);
	
	friend bool operator<= (const Rational<T>& lhs, const Rational<T>& rhs) { return !(lhs > rhs); }
	friend bool operator>= (const Rational<T>& lhs, const Rational<T>& rhs) { return !(lhs < rhs); }
	friend bool operator!= (const Rational<T>& lhs, const Rational<T>& rhs) { return !(lhs ==rhs); }
	
	/**
	 * Return a string representation of this rational, e.g. {@code "1/2"}.
	 *
	 * <p>The following rules of conversion apply:
	 * <ul>
	 * <li>{@code NaN} values will return {@code "nan"}
	 * <li>Positive infinity values will return {@code "+inf"}
	 * <li>Negative infinity values will return {@code "-inf"}
	 * <li>All other values will return {@code "numerator/denominator"} where {@code numerator} and
	 * {@code denominator} are substituted with the appropriate numerator and denominator values.
	 * </ul></p>
	 */
	template <typename charT, class traits>
	friend std::basic_ostream<charT, traits>& operator <<(
			std::basic_ostream<charT, traits>& os, const Rational<T>& r)
	{
		std::basic_ostringstream<charT, traits> stream;
		stream.flags(os.flags());
		stream.imbue(os.getloc());
		stream.precision(os.precision());
		
		if(r.den != 0)
		{
			if(r.num == 0)
				stream << (r.den > 0 ? '+' : '-') << '0';
			else
				stream << r.num << '/' << r.den;
		}
		else
		{
			if(r.num > 0)
				stream << "+inf";
			else if(r.num < 0)
				stream << "-inf";
			else
				stream << "nan";
		}
		
		return os << stream.str();
	}
};

template <typename T>
const Rational<T> Rational<T>::NaN(0, 0);

template <typename T>
const Rational<T> Rational<T>::POSITIVE_ZERO(0, +1);

template <typename T>
const Rational<T> Rational<T>::NEGATIVE_ZERO(0, -1);

template <typename T>
const Rational<T> Rational<T>::POSITIVE_INFINITY(+1, 0);

template <typename T>
const Rational<T> Rational<T>::NEGATIVE_INFINITY(-1, 0);

template <typename T>
constexpr Rational<T>::Rational(T n):
		num(n),
		den(static_cast<T>(1))
{
}

template <typename T>
constexpr Rational<T>::Rational(T numerator, T denominator):
		Rational(numerator, denominator, true/* normalize */)
{
}

template <typename T>
constexpr Rational<T>::Rational(T numerator, T denominator, bool normalize_):
		num(numerator),
		den(denominator)
{
	if(normalize_)
		normalize();
}

template <typename T>
constexpr T Rational<T>::gcd(T a, T b)
{
	auto abs = [](const T& x) { return x >= 0? x: -x; };
	// gcd(a, 0) := a
	// gcd(a, b) := gcd(b, a mod b)
#if 0  // recursive implementation
	if(a == 0)
		return abs(b);
	else if(b == 0)
		return abs(a);
	else
		return gcd(b, a % b);
#else  // non-recursive implementation
	while(b != 0)
	{
		T c = a % b;
		a = b;
		b = c;
	}
	return abs(a);
#endif
}

template <typename T>
constexpr void Rational<T>::normalize()
{
	if(den == 0)
	{
		if(num > 0)
			num = 1;  // +infity
		else if(num < 0)
			num = -1;  // -infity
//		else
//			num = 0;  // nan
		return;
	}
	
	if(den < 0)
	{
		if(den == -1 && num == 0)  // NEGATIVE_ZERO, the special case.
			return;
		
		num = -num;
		den = -den;
	}

	int32_t divisor = gcd(num, den);
	num /= divisor;
	den /= divisor;
}

template <typename T>
constexpr Rational<T> Rational<T>::reciprocal() const
{
#if 0
	return Rational<T> r(den, num);
#else
	Rational<T> r(den, num, false);
	// In floating point calculation, 1/-inf = -0.0, 1/nan = nan, 1/+inf = +0.0,
	if(r.den < -1 || (r.den == -1 && r.num != 0))
	{
		r.num = -r.num;
		r.den = -r.den;
	}
	return r;
#endif
}

template <typename T>
Rational<T>& Rational<T>::operator+= (const Rational<T>& other)
{
	if(den > 0 && other.den > 0)
	{
		// a/b + c/d = (a*d + b*c) / bd
		T g = gcd(this->den, other.den);
		den /= g;
		num = num * (other.den / g) + other.num * den;
		g = gcd(num, g);
		num /= g;
		den *= other.den / g;
	}
	else
	{
		if(isNaN() || other.isZero())
		{
			// nan + any = nan
			// any + zero = any
		}
		else if(isZero() || other.isNaN())
			*this = other;
		else if(other.isFinite())
		{
			// inf + finite = inf
		}
		else if(isFinite())
			*this = other;
		else if((isPositiveInfinity() && other.isNegativeInfinity()) ||
				(isNegativeInfinity() && other.isPositiveInfinity()))
			*this = NaN;  // +inf + -inf = nan
//		else  // inf + inf = inf, -inf + -inf = -inf
	}
	return *this;
}

template <typename T>
Rational<T>& Rational<T>::operator-= (const Rational<T>& other)
{
	*this += -other;
	return *this;
}

template <typename T>
Rational<T>& Rational<T>::operator*= (const Rational<T>& other)
{
	if(den != 0 && other.den != 0)
	{
		// a/b * c/d = (a*c) / (b*d), avoid overflow and preserve normalization.
		T d1 = gcd(num, other.den);
		T d2 = gcd(den, other.num);
		num = (num / d1) * (other.num / d2);
		den = (den / d2) * (other.den / d1);
	}
	else  // handle NaN and infinity cases
	{
		if(isNaN() || other.isNaN())  // nan * any = nan
			*this = Rational<T>::NaN;
		else  // inf * positive = inf, inf * 0 = nan, inf * negative = -inf
			*this = Rational<T>(num * other.num, 0);
	}
	return *this;
}

template <typename T>
Rational<T>& Rational<T>::operator/= (const Rational<T>& other)
{
	// (a/b) / (c/d) = (a*b) * (d/c)
	*this *= other.reciprocal();
	return *this;
}

template <typename T>
bool operator< (const Rational<T>& lhs, const Rational<T>& rhs)
{
	// NaN is unordered: it is not equal to, greater than, or less than anything, including itself.
	// x == x is false if and only if the value of x is NaN.
	T lhs_num = lhs.num, lhs_den = lhs.den;
	if(lhs_num == 0)
	{
		if(lhs_den == -1)
			lhs_den = 1;  // -0.0 => +0.0
		else if(lhs_den == 0)  // always return false when someone compares with NaN
			return false;
	}
	
	T rhs_num = rhs.num, rhs_den = rhs.den;
	if(rhs_num == 0)
	{
		if(rhs_den == -1)
			rhs_den = 1;
		else if(rhs_den == 0)
			return false;
	}
	
	if(lhs_den != 0 && rhs_den != 0)
	{
		// a/b < c/d  is equivalent to a*d < b*c when b > 0 & d > 0.
		// Notice that multipliation may overflows.
		T d1 = Rational<T>::gcd(lhs_num, rhs_num);
		T d2 = Rational<T>::gcd(lhs_den, rhs_den);
		T a = lhs_num / d1, c = rhs_num / d1;
		T b = lhs_den / d2, d = rhs_den / d2;
		return a * d < b * c;
	}
	
	// at least one is infinity
	if(lhs_den == 0)
	{
		if(rhs_den == 0)  // both are infinities, -inf < +inf, +inf is not less than +inf
			return lhs_num < rhs_num;
		else  // lhs is infinity, -inf < finite < +inf, negative sign returns true
			return lhs_num < 0;
	}
	else  // rhs is infinity, -inf < finite < +inf, positive sign return true
		return 0 < rhs_num;
}

template <typename T>
bool operator> (const Rational<T>& lhs, const Rational<T>& rhs)
{
	// NaN is unordered: it is not equal to, greater than, or less than anything, including itself.
	// x == x is false if and only if the value of x is NaN.
	T lhs_num = lhs.num, lhs_den = lhs.den;
	if(lhs_num == 0)
	{
		if(lhs_den == -1)
			lhs_den = 1;  // -0.0 => +0.0
		else if(lhs_den == 0)  // always return false when someone compares with NaN
			return false;
	}
	
	T rhs_num = rhs.num, rhs_den = rhs.den;
	if(rhs_num == 0)
	{
		if(rhs_den == -1)
			rhs_den = 1;
		else if(rhs_den == 0)
			return false;
	}
	
	if(lhs_den != 0 && rhs_den != 0)
	{
		// a/b > c/d  is equivalent to a*d > b*c when b > 0 & d > 0.
		T d1 = Rational<T>::gcd(lhs_num, rhs_num);
		T d2 = Rational<T>::gcd(lhs_den, rhs_den);
		T a = lhs_num / d1, c = rhs_num / d1;
		T b = lhs_den / d2, d = rhs_den / d2;
		return a * d > b * c;
	}
	
	// at least one is infinity
	if(lhs_den == 0)
	{
		if(rhs_den == 0)  // both are infinities, +inf > -inf, +inf is not less than +inf
			return lhs_num > rhs_num;
		else  // lhs is infinity, +inf > finite > -inf, positive sign returns true
			return lhs_num > 0;
	}
	else  // rhs is infinity, +inf > finite > -inf, negative sign return true
		return 0 > rhs_num;

}

template <typename T>
bool operator== (const Rational<T>& lhs, const Rational<T>& rhs)
{
	if(lhs.den != 0 && rhs.den != 0)
		return lhs.num == rhs.num && (lhs.num == 0 || lhs.den == rhs.den);
	
	// handle NaN and infinity cases.
	if(lhs.isNaN() || rhs.isNaN())  // always return false when someone compares with NaN
		return false;
	
	return lhs.num == rhs.num;
}

}  // namespace pea
#endif  // PEA_UTIL_RATIONAL_H_
