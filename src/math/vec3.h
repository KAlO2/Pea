#ifndef PEA_MATH_VEC3_H_
#define PEA_MATH_VEC3_H_

#include <cmath>
#include <iomanip>
#include <sstream>

#include "math/scalar.h"

namespace pea {

template <typename T>
class vec3
{
public:
// https://msdn.microsoft.com/en-us/library/2c8f766e.aspx
#ifdef _WIN32
#  pragma warning(push)
#  pragma warning(disable: 4201)  // warning C4201: nonstandard extension used: nameless struct/union
#endif

	union
	{
		T data[3];
		struct { T x, y, z; };  // vertex
		struct { T r, g, b; };  // color
		struct { T u, v, w; };  // texture
		struct { T s, t, p; };  // texture
		struct { T i, j, k; };  // index
		struct { T start, step, stop; }; // arithmetic progression
		struct { T pitch, roll, yaw; };
		
		// http://en.wikipedia.org/wiki/Spherical_coordinate_system
		// rho: distance of from the origin, [0, +inf)
		// theta: the angle project to XOY-plane, in a counter-clockwise direction. [-pi, pi]
		// phi: the reference angle from z-axis. [-pi/2, +pi/2]
		struct { T rho, theta, phi; };  // polar coordinate
		struct { T width, height, depth; };  // cube
		struct { T constant, linear, quadratic; };  // attenuation
	};

#ifdef _WIN32
#  pragma warning(pop)
#endif

	typedef T value_type;

public:
	vec3()  = default;
	~vec3() = default;

	explicit constexpr vec3(const T *array): x(array[0]), y(array[1]), z(array[2]) { }
	explicit constexpr vec3(T scalar): x(scalar), y(scalar), z(scalar) { }
	constexpr vec3(T x, T y, T z): x(x), y(y), z(z) { }

	constexpr vec3(const vec3<T>& v) = default;  //: x(v.x), y(v.y), z(v.z) { }
	vec3<T>& operator =(const vec3<T>& other) = default;
	
	const T& operator [](size_t n) const { return data[n]; }
	      T& operator [](size_t n)       { return data[n]; }

	// assignment operators
	vec3<T>& operator +=(const vec3<T>& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
	vec3<T>& operator -=(const vec3<T>& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
	vec3<T>& operator *=(T value)            { x *= value; y *= value; z *= value; return *this; }
	vec3<T>& operator /=(T value)            { x /= value; y /= value; z /= value; return *this; }

	// unary operators
	vec3<T> operator +() const { return vec3<T>(+x, +y, +z); }
	vec3<T> operator -() const { return vec3<T>(-x, -y, -z); }

	// binary operators, multiply and divide operations for scalar
	friend vec3<T> operator +(const vec3<T>& lhs, const vec3<T>& rhs) { vec3<T> result(lhs); result += rhs; return result; }
	friend vec3<T> operator -(const vec3<T>& lhs, const vec3<T>& rhs) { vec3<T> result(lhs); result -= rhs; return result; }
//	friend vec3<T> operator *(const vec3<T>& lhs, const vec3<T>& rhs) { return vec3<T>(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z); }
	friend vec3<T> operator *(const vec3<T>& lhs, T value) { return vec3<T>(lhs.x * value, lhs.y * value, lhs.z * value); }
	friend vec3<T> operator *(T value, const vec3<T>& rhs) { return vec3<T>(rhs.x * value, rhs.y * value, rhs.z * value); }
	friend vec3<T> operator /(const vec3<T>& lhs, T value) { return vec3<T>(lhs.x / value, lhs.y / value, lhs.z / value); }

	friend bool operator <(const vec3<T>& lhs, const vec3<T>& rhs)
	{
		// Operator < and strict weak ordering
		if(lhs.x != rhs.x)
			return lhs.x < rhs.x;
		if(lhs.y != rhs.y)
			return lhs.y < rhs.y;
		if(lhs.z != rhs.z)
			return lhs.z < rhs.z;

		return false;
	}

	friend bool operator ==(const vec3<T>& lhs, const vec3<T>& rhs)
	{
		return fuzzyEqual(lhs.x, rhs.x) && fuzzyEqual(lhs.y, rhs.y) && fuzzyEqual(lhs.z, rhs.z);
	}
	
	friend bool operator !=(const vec3<T>& lhs, const vec3<T>& rhs)
	{
		return !(lhs == rhs);
	}

	T length2() const { return x*x + y*y + z*z; }
	T length() const { return std::sqrt(x*x + y*y + z*z); }
	vec3<T>& normalize() { *this /= length(); return *this; }

	friend vec3<T> normalize(const vec3<T>& v)
	{
		return vec3<T>(v).normalize();
	}
	
	friend vec3<T> abs(const vec3<T>& v)
	{
		return vec3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
	}
	
	friend vec3<T> lerp(const vec3<T>& v0, const vec3<T>& v1, const T& t)
	{
		T x = lerp(v0.x, v1.x, t);
		T y = lerp(v0.y, v1.y, t);
		T z = lerp(v0.z, v1.z, t);
		return vec3<T>(x, y, z);
	}
	
	/**
	 * Give the position a spherical coordinate system(rho, theta, phi) view.
	 *
	 * @param[in] position Cartesian coordinate.
	 * @return polar coordinate where rho >= 0, theta in [-pi, +pi], phi in [-pi/2, pi/2].
	 */
	friend vec3<T> polar_cast(const vec3<T>& position)
	{
		vec3<T> polar(static_cast<T>(0));
		polar.rho = position.length();
		if(!isZero<T>(polar.rho))
		{
			polar.phi = std::asin(position.z / polar.rho);
			polar.theta = std::atan2(position.y, position.x);
		}
		return polar;
	}

	friend vec3<T> cartesian_cast(const vec3<T>& polar)
	{
		assert(polar.rho >= 0);
		assert(-M_PI / 2 <= polar.phi && polar.phi <= +M_PI / 2);
		vec3<T> position;
		position.z = polar.rho * std::sin(polar.phi);
		T r = polar.rho * std::cos(polar.phi);
		position.x = r * std::cos(polar.theta);
		position.y = r * std::sin(polar.theta);
		return position;
	}

	/**
	 * @param[in] direction vector, normalized value.
	 * 
	 */
	vec3<T> project(const vec3<T>& direction) const
	{
		assert(fuzzyEqual<T>(direction.length2(), 1));
		return dot(*this, direction) * direction;
	}

	vec3<T>& translate(const vec3<T>& vector)
	{
		*this += vector;
		return *this;
	}

	/**
	 * @brief It's equivalent to left multiplying matrix
	 *
	 * [1     0           0     ]
	 * [0 cos(angle) -sin(angle)]
	 * [0 sin(angle)  cos(angle)]
	 */
	vec3<T>& rotateX(T angle)
	{
		T c = std::cos(angle);
		T s = std::sin(angle);
		T _y = c * y - s * z;
		T _z = s * y + c * z;

		y = _y;
		z = _z;
		return *this;
	}

	/**
	 * @brief It's equivalent to left multiplying matrix
	 *
	 * [ cos(angle) 0 sin(angle)]
	 * [    0       1     0     ]
	 * [-sin(angle) 0 cos(angle)]
	 */
	vec3<T>& rotateY(T angle)
	{
		T c = std::cos(angle);
		T s = std::sin(angle);
		T _x = s * z + c * x;
		T _z = c * z - s * x;

		x = _x;
		z = _z;
		return *this;
	}

	/**
	 * @brief It's equivalent to left multiplying matrix
	 *
	 * [cos(angle) -sin(angle) 0]
	 * [sin(angle)  cos(angle) 0]
	 * [    0           0      1]
	 */
	vec3<T>& rotateZ(T angle)
	{
		T c = std::cos(angle);
		T s = std::sin(angle);
		T _x = c * x - s * y;
		T _y = s * x + c * y;

		x = _x;
		y = _y;
		return *this;
	}

	vec3<T>& rotate(const vec3<T>& v, T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle), C = 1-c;
		const T &x = v.x, &y = v.y, &z = v.z;

		T _x = (   c + x*x*C)*this->x + (-z*s + x*y*C)*this->y + ( y*s + x*z*C)*this->z;
		T _y = ( z*s + y*x*C)*this->x + (   c + y*y*C)*this->y + (-x*s + y*z*C)*this->z;
		T _z = (-y*s + z*x*C)*this->x + ( x*s + z*y*C)*this->y + (   c + z*z*C)*this->z;

		this->x = _x;
		this->y = _y;
		this->z = _z;
		return *this;
	}

	friend T angle(const vec3<T>& v1, const vec3<T>& v2)
	{
		T numerator = dot(v1, v2);
		T denominator = v1.length() * v2.length();
		assert(!isZero<T>(denominator));
		return std::acos(numerator / denominator);
	}

	friend T distance(const vec3<T>& pt1, const vec3<T>& pt2)
	{
		vec3<T> v = pt2 - pt1;
		return v.length();
	}

	friend T dot(const vec3<T>& v1, const vec3<T>& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	friend vec3<T> cross(const vec3<T>& v1, const vec3<T>& v2)
	{
/*
		|  i    j    k   |
		| v1.x v1.y v1.z |
		| v2.x v2.y v2.z |
*/
		return vec3<T>(
			v1.y * v2.z - v2.y * v1.z,
			v1.z * v2.x - v2.z * v1.x,
			v1.x * v2.y - v2.x * v1.y);
	}

	template <typename charT, class traits>
	friend std::basic_istream<charT, traits>& operator >>(
			std::basic_istream<charT, traits>& is, vec3<T>& v)
	{
		T _x, _y, _z;
		charT ch0, ch1, ch2, ch3;

		// only '(_x, _y, _z)' format is allowed
		is >> ch0 >> _x >> ch1 >> _y >> ch2 >> _z >> ch3;
		if(ch0 == '(' && ch1 == ',' && ch2 == ','  && ch3 == ')')
			v = vec3<T>(_x, _y, _z);
		else
			is.setstate(std::ios_base::failbit);

		return is;
	}

	template <typename charT, class traits>
	friend std::basic_ostream<charT, traits>& operator <<(
			std::basic_ostream<charT, traits>& os, const vec3<T>& v)
	{
		std::basic_ostringstream<charT, traits> stream;
		stream.flags(os.flags());
		stream.imbue(os.getloc());
		stream.precision(os.precision());
		stream << '(' << v.x << ", " << v.y << ", " << v.z << ')';
		return os << stream.str();
	}
};

using vec3i = vec3<int32_t>;
using vec3u = vec3<uint32_t>;
using vec3f = vec3<float>  ;
using vec3d = vec3<double> ;

}  // namespace pea
#endif  // PEA_MATH_VEC3_H_
