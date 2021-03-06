#ifndef PEA_MATH_QUATERNION_H_
#define PEA_MATH_QUATERNION_H_

#include <complex>

#include "math/vec3.h"
#include "math/mat3.h"

namespace pea {

/**
 * Quaternions consist of 4 values w, x, y, z, where w is the real (scalar) part, and x, y, z are the complex (vector) part.
 * Here, we care more about unit quaternions, also known as versors, provide a convenient mathematical notation for
 * representing orientations and rotations of objects in three dimensions. Compared to
 * Euler angles they are simpler to compose and avoid the problem of gimbal lock. Compared
 * to rotation matrices they are more numerically stable and may be more efficient.
 * see {@link http://en.wikipedia.org/wiki/Quaternion }
 * see {@link http://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation }
 * see Ken Shoemake {@link http://www.cs.ucr.edu/~vbz/resources/quatut.pdf }
 * see {@link http://www.essentialmath.com/GDC2013/GDC13_quaternions_final.pdf }
 */
template<typename T>
class quaternion
{
public:
	// Using WXYZ or XYZW layout doesn't matter, some prefer one than another.
	// q = scalar + vector = w + x*i + y*j + z*k
	T x, y, z, w;

public:
	constexpr quaternion(): quaternion(T(0), T(0), T(0), T(1)) {}
	constexpr quaternion(const T& x, const T& y, const T& z, const T& w): x(x), y(y), z(z), w(w) {}
	constexpr quaternion(const T& scalar, const vec3<T>& vector): x(vector.x), y(vector.y), z(vector.z), w(scalar) {}
	
	// copy constructor and destructor, the compiler can handle it well.
	constexpr quaternion(const quaternion<T>& other) = default;
	~quaternion() = default;
	
	/**
	 * Construct a quaternion from the Euler rotation angles (x, y, z) a.k.a. (pitch, roll, yaw).
	 * Note that we are applying in order: (x, y, z) a.k.a. (pitch, roll, yaw).
	 * @param gimbal a x, y, z angle tuple that represents in radians.
	 */
	quaternion(const vec3<T>& gimbal)
	{
		vec3<T> theta = gimbal / 2;
		vec3<T> sinV(std::sin(theta.x), std::sin(theta.y), std::sin(theta.z));
		vec3<T> cosV(std::cos(theta.x), std::cos(theta.y), std::cos(theta.z));

		// variables used to reduce multiplication calls.
		T cosYcosZ = cosV.y * cosV.z, sinYsinZ = sinV.y * sinV.z;
		T cosYsinZ = cosV.y * sinV.z, sinYcosZ = sinV.y * cosV.z;

		w = (cosYcosZ * cosV.x - sinYsinZ * sinV.x);
		x = (cosYcosZ * sinV.x + sinYsinZ * cosV.x);
		y = (sinYcosZ * cosV.x + cosYsinZ * sinV.x);
		z = (cosYsinZ * cosV.x - sinYcosZ * sinV.x);

		normalize();
	}

	/**
	 * Construct a unit quaternion.
	 * A rotation about the unit vector N by an angle θ can be written as
	 * q = (s, v) = (cos(θ/2), sin(θ/2)*n)
	 */
	quaternion(const vec3<T>& normal, T theta)
	{
		theta /= 2;
		w = std::cos(theta);

		assert(fuzzyEqual<T>(T(1), normal.length()));
		T s = std::sin(theta);
		x = s * normal.x;
		y = s * normal.y;
		z = s * normal.z;
	}

	/**
	 * construct a unit quaternion from two complex numbers
	 *
	 * (a, b, c, d) = a + bi +cj + dk = (a+bi)+(c+di)j = ((a, b), (c, d))
	 * See, "two dimensional" complex number! By saying two, I mean i != j.
	 *
	 * The quaternions can be represented using complex 2×2 matrices
	 * H =
	 * 	 [ a+bi, c+di]
	 * 	 [-c+di, a-bi]
	 *
	 * Quaternions can also be represented using the complex 2×2 matrices
	 * U =      I =       J =       K =
	 *   [1 0]    [i  0]    [ 0 1]    [0 i]
	 *   [0 1]    [0 -i]    [-1 0]    [i 0]
	 *
	 * or real 4x4 matrices
	 * U =            I =            J =            K =
	 *   [1 0 0 0]      [ 0 1  0 0]    [0 0  0 -1]    [0  0 -1 0]
	 *   [0 1 0 0]      [-1 0  0 1]    [0 0 -1  0]    [0  0  0 1]
	 *   [0 0 1 0]      [ 0 0  0 1]    [0 1  0  0]    [1  0  0 0]
	 *   [0 0 0 1]      [ 0 0 -1 0]    [1 0  0  0]    [0 -1  0 0]
	 *
	 * Q = wI + xI + yJ + zK behaves just like a quaternion, for they satisfy the following rules.
	 * U^2 = 1, I^2 = J^2 = K^2 = -1,
	 * IJ = -JI = K, JK = -KJ = I, KI = -IK = J.
	 */
	constexpr quaternion(const std::complex<T>& a, const std::complex<T>& b):
			w(a.real()), x(a.imag()),
			y(b.real()), z(b.imag())
	{
	}

	constexpr quaternion<T>& operator =(const quaternion<T>& rhs)
	{
		if(this != &rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;
			w = rhs.w;
		}
		return *this;
	}

	constexpr quaternion<T>& operator +=(const T& s) { w += s; return *this; }
	constexpr quaternion<T>& operator -=(const T& s) { w -= s; return *this; }
	constexpr quaternion<T>& operator *=(const T& s) { w *= s; x *= s; y *= s; z *= s; return *this; }
	constexpr quaternion<T>& operator /=(const T& s) { w /= s; x /= s; y /= s; z /= s; return *this; }

	friend constexpr quaternion<T> operator +(const quaternion<T>& lhs, const quaternion<T>& rhs)
	{
		quaternion<T> q;
		q.x = lhs.x + rhs.x;
		q.y = lhs.y + rhs.y;
		q.z = lhs.z + rhs.z;
		q.w = lhs.w + rhs.w;
		return q;
	}
	
	friend constexpr quaternion<T> operator *(const T& s, const quaternion<T>& q)
	{
		quaternion<T> ret;
		ret.x = s * q.x;
		ret.y = s * q.y;
		ret.z = s * q.z;
		ret.w = s * q.w;
		return ret;
	}

	friend constexpr quaternion<T> operator *(const quaternion<T>& lhs, const quaternion<T>& rhs)
	{
		quaternion<T> result(lhs);
		result *= rhs;
		return result;
	}

	/**
	 * note that quaternion multiplication is *NOT* commutative;
	 * symbolically, "q *= p;" means "q = q * p;"
	 * and "q /= p;" means "q = q * inverse_of(p);"
	 */
	constexpr quaternion<T>& operator +=(const quaternion<T>& q)
	{
		w += q.w;
		x += q.x;
		y += q.y;
		z += q.z;
		return *this;
	}

	constexpr quaternion<T>& operator -=(const quaternion<T>& q)
	{
		w -= q.w;
		x -= q.x;
		y -= q.y;
		z -= q.z;
		return *this;
	}

	/**
	 * q1 = s1 + v1, q2 = s2 + v2, where s is scalar component, v is vector component
	 * q1q2 = (s1, v1)*(s2, v2) = (s1*s2 - dot(v1, v2), s1*v2 + s2*v1 + cross(v1*v2))
	 */
	constexpr quaternion<T>& operator *=(const quaternion<T>& q)
	{
/*
		T _w = w*q.w - x*q.x - y*q.y - z*q.z;
		T _x = w*q.x + x*q.w + y*q.z - z*q.y;
		T _y = w*q.y - x*q.z + y*q.w + z*q.x;
		T _z = w*q.z + x*q.y - y*q.x + z*q.w;

		w = _w;
		x = _x;
		y = _y;
		z = _z;
*/
		vec3<T> v(x, y, z), q_v(q.x, q.y, q.z);
		T       _w = w * q.w - dot(v, q_v);
		vec3<T> _v = w * q_v + q.w * v + cross(v, q_v);

		x = _v.x;
		y = _v.y;
		z = _v.z;
		w = _w;

		return *this;
	}

	constexpr quaternion<T>& operator /=(const quaternion<T>& q)
	{
		// the reciprocal of q is q^(-1) = conj(q)/(norm(q)*norm(q));
		T _w = x*q.z + y*q.w + z*q.x + w*q.y;
		T _x = x*q.w - y*q.z - z*q.y + w*q.x;
		T _y = x*q.x + y*q.y - z*q.z - w*q.w;
		T _z = x*q.y - y*q.x + z*q.w - w*q.z;

		const T norm = x*x +y*y + z*z + w*w;
		w = _w / norm;
		x = _x / norm;
		y = _y / norm;
		z = _z / norm;

		return *this;
	}

	friend constexpr bool operator ==(const quaternion<T>& q1, const quaternion<T>& q2)
	{
		return fuzzyEqual(q1.w, q2.w) &&
				fuzzyEqual(q1.x, q2.x) && fuzzyEqual(q1.y, q2.y) && fuzzyEqual(q1.z, q2.z);
	}
	
	friend constexpr bool operator !=(const quaternion<T>& q1, const quaternion<T>& q2)
	{
		return !(q1 == q2);
	}
	
	/**
	 * cast a quaternion to 3x3 matrix, note that rotation matrices here apply to column vectors.
	 */
	constexpr mat3<T> mat3_cast() const
	{
		T xx = x*x, yy = y*y, zz = z*z;
		T xy = x*y, yz = y*z, xz = x*z;
		T wx = w*x, wy = w*y, wz = w*z;

		return mat3<T>(
			1 - 2*(yy + zz),     2*(xy - wz),     2*(xz + wy),
			    2*(xy + wz), 1 - 2*(xx + zz),     2*(yz - wx),
			    2*(xz - wy),     2*(yz + wx), 1 - 2*(xx + yy));
	}
/*
	mat4<T> mat4_cast() const
	{
		T xx = x*x, yy = y*y, zz = z*z;
		T xy = x*y, yz = y*z, xz = x*z;
		T wx = w*x, wy = w*y, wz = w*z;

		return mat4<T>(
		1 - 2*(yy + zz),     2*(xy - wz),     2*(xz + wy), 0,
		    2*(xy + wz), 1 - 2*(xx + zz),     2*(yz - wx), 0,
		    2*(xz - wy),     2*(yz + wx), 1 - 2*(xx + yy), 0,
			          0,               0,               0, 1);

		// or an elegant way to express
		mat4 A(
			 w,  z,  y,  x,
			-z,  w,  x,  y,
			 y, -x,  w,  z,
			-x, -y, -z,  w);
		mat4 B(
			 w,  z, -y, -x,
			-z,  w,  x, -y,
			 y, -x,  w, -z,
			 x,  y,  z,  w);
		return A*B;
	}
*/
	// TODO havn't test yet.
	vec3<T> eularAngle() const
	{
		T xx = x*x, yy = y*y, zz = z*z, ww = w*w;
		T norm = xx + yy + zz + ww;

		T test = x*y + z*w;
		constexpr T HALF = static_cast<T>(0.499);  // for calculation error
		if(test > HALF * norm)  // singularity at north pole
			return vec3<T>(0, 2*std::atan2(x, w), M_PI/2);
		else if (test < -HALF * norm)  // singularity at south pole
			return vec3<T>(0, -2*std::atan2(x, w), -M_PI/2);
		else
		{
			T yaw = std::atan2(2*x*w - 2*y*z, -xx + yy - zz + ww);
			T roll = std::atan2(2*y*w - 2*x*z, xx - yy - zz + ww);
			T pitch = std::asin(2*test/norm);
			return vec3<T>(yaw, roll, pitch);
		}
	}

	quaternion<T> inverse() const
	{
		quaternion<T> conj = conjugate();
		T len = length();
		return conj / len;
	}

	/**
	 * The conjugate of q is the quaternion q* = w -xi - yj -zk.
	 * Note that if p and q are quaternions, then (pq)∗ = q∗p∗, not p∗q∗.
	 */
	inline constexpr quaternion<T> conjugate() const
	{
		return quaternion<T>(-x, -y, -z, w);
	}

	/**
	 * Rotating p by q refers to the operation p => qpq^−1
	 * If q is a versor, namely unit quternion. q^-1 = q.conjugate(), and p = qpq*
	 */
	quaternion<T> rotate(const quaternion<T>& q) const
	{
		quaternion<T> unit = q;
		unit.normalize();
		return unit * (*this) * unit.conjugate();
	}

	inline T length() const
	{
		return std::sqrt(w*w + x*x + y*y + z*z);
	}

	quaternion<T>& normalize()
	{
		*this /= length();
		return *this;
	}

	/**
	 * non-commutative property makes the quaternions exp(p) * exp(q) and exp(p + q) are not
	 * necessarily equal.
	 */
/*	static quaternion<T> exp(const quaternion<T>& q) const
	{
		T len = length();
		quaternion<T> q2 = q/len;
		T theta = std::acos(q2.w);
		vec3<T> v = q2.v / std::sin(theta);
		q2.w = std::cos(x * theta);
		q2.v = v * std::sin(x * theta);

		return std::exp(len) * q2;
	}

	static quaternion<T> log(const quaternion<T>& q) const
	{
		T len = length();
		quaternion<T> q2 = q/len;
		T theta = std::acos(q2.w);
		vec3<T> v = q2.v / std::sin(theta);
		q2.w = std::cos(x * theta);
		q2.v = v * std::sin(x * theta);

		return std::log(len) * q2;
	}
*/
	static quaternion<T> slerp(const quaternion<T>& q0, const quaternion<T>& q1, T t)
	{
		static_assert(std::is_floating_point<T>::value, "limited to scalar floating-point type only");

		T cos_omega = dot(q0, q1);
		T one_minus_t = 1.0 - t;

		if(cos_omega < static_cast<T>(1.0 - 1.0E-6))
		{
			T omega = std::acos(cos_omega);
			T sin_omega = std::sin(omega); // sqrt(1-cos_omega*cos_omega);

			one_minus_t = std::sin(one_minus_t * omega) / sin_omega;
			t = std::sin(t * omega) / sin_omega;
		}
		else
		{
			// 'from' and 'to' are too close for comfort
			// just do linear interpolation, speed up!
		}

		return one_minus_t * q0 + t * q1;
	}

	friend constexpr T dot(const quaternion<T>& lhs, const quaternion<T>& rhs)
	{
		return lhs.w*rhs.w + lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
	}

	template<typename charT, class traits>
	friend std::basic_istream<charT, traits>& operator >>(
			std::basic_istream<charT, traits>& is, quaternion<T>& q)
	{
		// only accept "w + x*i + y*j + z*k" pattern
		T w, x, y, z;
		charT i, j, k;

		is >> w >> x >> i >> y >> j >> z >> k;
		if(i == 'i' && j == 'j' && k == 'k')
			q = quaternion<T>(w, x, y, z);
		else
		{
			q = quaternion<T>(w);
			is.setstate(std::ios_base::failbit);
//			is.putback(ch);
		}

		return is;
	}

	template<typename charT, class traits>
	friend std::basic_ostream<charT, traits>& operator <<(
			std::basic_ostream<charT, traits>& os, const quaternion<T>& q)
	{
		std::basic_ostringstream<charT, traits> s;
		s.flags(os.flags());
		s.imbue(os.getloc());
		s.precision(os.precision());
//		s << '(' << q.w << ',' << q.x << ',' << q.y << ',' << q.z << ')';
		s << q.w << std::showpos << q.x << 'i' << q.y << 'j' << q.z << 'k';
		return os << s.str();
	}
};

using quaternionf = quaternion<float>;
using quaterniond = quaternion<double>;

}  // namespace pea
#endif  // PEA_MATH_QUATERNION_H_
