#ifndef PEA_MATH_VEC2_H_
#define PEA_MATH_VEC2_H_

#include <iomanip>
#include <sstream>

#include "math/scalar.h"

namespace pea {

template <typename T>
class alignas(2 * sizeof(T)) vec2
{
public:

// https://msdn.microsoft.com/en-us/library/2c8f766e.aspx
#ifdef _WIN32
#  pragma warning(push)
#  pragma warning(disable: 4201)  // warning C4201: nonstandard extension used: nameless struct/union
#endif
	union
	{
		T data[2];
		struct { T x, y; }; // 2D coordinate
		struct { T i, j; }; // index, integer pair
		struct { T s, t; }; // texture
		struct { T u, v; }; // texture
		
		struct { T gray, alpha;   }; // color
		struct { T rho, theta;    }; // polar coordinate
		struct { T width, height; }; // size
		struct { T column, row;   }; // grid, index, notice the order
		struct { T min, max;      }; // 1D AABB
		struct { T slice, stack;  }; // cut shape
	};

#ifdef _WIN32
#  pragma warning(pop)
#endif

	typedef T value_type;

public:
	vec2()  = default;
	~vec2() = default;

	explicit constexpr vec2(const T *array): x(array[0]), y(array[1]) { }
	explicit constexpr vec2(T scalar): x(scalar), y(scalar) { }
	constexpr vec2(T x, T y): x(x), y(y) { }

	constexpr vec2(const vec2<T>& v) = default; //: x(v.x), y(v.y) {}

	vec2<T>& operator =(const vec2<T>& rhs)
	{
		if(this != &rhs)
		{
			x = rhs.x;
			y = rhs.y;
		}
		return *this;
	}

	const T& operator [](size_t n) const { return data[n]; }
	      T& operator [](size_t n)       { return data[n]; }
	
	// assignment operators
	vec2<T>& operator +=(const vec2<T>& rhs) { x += rhs.x; y += rhs.y; return *this; }
	vec2<T>& operator -=(const vec2<T>& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	vec2<T>& operator *=(T value)            { x *= value; y *= value; return *this; }
	vec2<T>& operator /=(T value)            { x /= value; y /= value; return *this; }

	/**
	 * @brief unary operators
	 * @{
	 */
	inline vec2<T> operator +() const { return vec2<T>(+x, +y); }
	inline vec2<T> operator -() const { return vec2<T>(-x, -y); }
	/** @} */

	/**
	 * binary operators, add subtract, multiply and divide operations for scalar
	 * @{
	 */
	friend vec2<T> operator +(const vec2<T>& lhs, const vec2<T>& rhs) { vec2<T> result(lhs); result += rhs; return result; }
	friend vec2<T> operator -(const vec2<T>& lhs, const vec2<T>& rhs) { vec2<T> result(lhs); result -= rhs; return result; }
//	friend vec2<T> operator *(const vec2<T>& lhs, const vec2<T>& rhs) { return vec2<T>(lhs.x * rhs.x, lhs.y * rhs.y); }
	friend vec2<T> operator *(const vec2<T>& lhs, T value) { return vec2<T>(lhs.x * value, lhs.y * value); }
	friend vec2<T> operator *(T value, const vec2<T>& rhs) { return vec2<T>(rhs.x * value, rhs.y * value); }
	friend vec2<T> operator /(const vec2<T>& lhs, T value) { return vec2<T>(lhs.x / value, lhs.y / value); }
	/** @} */

	friend bool operator <(const vec2<T>& lhs, const vec2<T>& rhs)
	{
		if(lhs.x != rhs.x)
			return lhs.x < rhs.x;
		return lhs.y < rhs.y;
	}

	friend bool operator ==(const vec2<T>& lhs, const vec2<T>& rhs)
	{
		return fuzzyEqual(lhs.x, rhs.x) && fuzzyEqual(lhs.y, rhs.y);
	}
	
	friend bool operator !=(const vec2<T>& lhs, const vec2<T>& rhs)
	{
		return !(lhs == rhs);
	}

	T length2() const { return x*x + y*y; }
	T length() const { return std::sqrt(x*x + y*y); }
	vec2<T>& normalize() { *this /= length(); return *this; }

	friend vec2<T> normalize(const vec2<T>& v)
	{
		return vec2<T>(v).normalized();
	}
	
	friend vec2<T> abs(const vec2<T>& v)
	{
		return vec2<T>(std::abs(v.x), std::abs(v.y));
	}
	
	friend vec2<T> lerp(const vec2<T>& v0, const vec2<T>& v1, const T& t)
	{
		T x = lerp(v0.x, v1.x, t);
		T y = lerp(v0.y, v1.y, t);
		return vec2<T>(x, y);
	}
	
	/**
	 * polar coordinate system (rho, theta)
	 *
	 * @param[in] v coordinate system
	 * @return polar coordinate system (rho, theta). Rho is the magnitude of the vector, theta is the 
	 *         return value is in the range [-pi, pi].
	 */
	vec2<T> polar() const
	{
		T _rho = length();
		T _theta = std::atan2(y, x);
		return vec2<T>(_rho, _theta);
	}

	vec2<T> project(vec2<T>& direction) const
	{
		assert(isZero<T>(length2() - 1));
		return dot(*this, direction) * direction;
	}

	vec2<T>& translate(const vec2<T>& vector)
	{
		*this += vector;
		return *this;
	}

	vec2<T>& rotate(T angle)
	{
		// complex<T>(x,y) * complex<T>(cos(angle), sin(angle));
		const T c = std::cos(angle);
		const T s = std::sin(angle);
		T _x = c * x - s * y;
		T _y = s * x + c * y;

		x = _x;
		y = _y;
		return *this;
	}

	friend T angle(const vec2<T>& v1, const vec2<T>& v2)
	{
		T numerator = dot(v1, v2);
		T denominator = v1.length() * v2.length();
//		assert(!isZero<T>(denominator));
		return std::acos(numerator / denominator);
	}

	friend T distance(const vec2<T>& pt1, const vec2<T>& pt2)
	{
		vec2<T> v = pt2 - pt1;
		return v.length();
	}

	friend T dot(const vec2<T>& v1, const vec2<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	/**
	 * The cross product of the two vectors in 2D.
	 * If the result is 0, the points are colinear; if it is positive, the two
	 * vectors constitute a "left turn" or counter-clockwise orientation,
	 * otherwise a "right turn" or clockwise orientation.
	 *
	 * http://www.euclideanspace.com/maths/algebra/vectors/vecGeometry/vec2d/index.htm
	 * This may be useful to calculate triangle's directed area, which has both
	 * quantity and direction.
	 */
	friend T cross(const vec2<T>& v1, const vec2<T>& v2)
	{
/*
		|  i    j   k |
		| v1.x v1.y 0 |
		| v2.x v2.y 0 |
*/
		return v1.x * v2.y - v2.x * v1.y;
	}

	template <typename charT, class traits>
	friend std::basic_istream<charT, traits>& operator >>(
			std::basic_istream<charT, traits>& is, vec2<T>& v)
	{
/*
		charT ch;
		is >> ch;
		if(ch == '(')
		{
			is >> _x >> ch;
			if(ch == ',')
			{
				is >> _y >> ch;
				if(ch == ')')
					v = vec2<U>(_x, _y);
				else
					is.setstate(std::ios_base::failbit);
			}
			else if(ch == ')')
				v = _x;
			else
				is.setstate(std::ios_base::failbit);
		}
		else
		{
			is.putback(ch);
			is >> _x;
			v = _x;
		}
*/
		T _x, _y;
		charT ch0, ch1, ch2;

		// only '(_x, _y)' format is allowed
		is >> ch0 >> _x >> ch1 >> _y >> ch2;
		if(ch0 == '(' && ch1 == ',' && ch2 == ')')
			v = vec2<T>(_x, _y);
		else
			is.setstate(std::ios_base::failbit);

		return is;
	}

	template <typename charT, class traits>
	friend std::basic_ostream<charT, traits>& operator <<(
			std::basic_ostream<charT, traits>& os, const vec2<T>& v)
	{
		std::basic_ostringstream<charT, traits> stream;
		stream.flags(os.flags());
		stream.imbue(os.getloc());
		stream.precision(os.precision());
		stream << '(' << v.x << ", " << v.y << ')';
		return os << stream.str();
	}

};

using vec2i = vec2<int32_t>;
using vec2u = vec2<uint32_t>;
using vec2f = vec2<float>  ;
using vec2d = vec2<double> ;

}  // namespace pea
#endif  // PEA_MATH_VEC2_H_
