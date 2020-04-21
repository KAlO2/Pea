#ifndef PEA_MATH_MAT3X4_H_
#define PEA_MATH_MAT3X4_H_

#include <iostream>

#include "math/mat3.h"
#include "math/quaternion.h"


namespace pea {

/**
 * @class mat3x4 is composition of translation/rotation/scaling operations. (affine transform)
 * In column-major mode, left multiplying is used; while in row-major mode, right multiplying
 * is used. As such, a transform A followed by a transform B can be written B*A in column-
 * major mode, or A*B in row-major mode.
 */
template <typename T>
class mat3x4
{
	/* matrix
	 * column major order            row major order
	 * [_0, _4, _8 , _12]          [_0 , _1 , _2 , _3 ]
	 * [_1, _5, _9 , _13]          [_4 , _5 , _6 , _7 ]
	 * [_2, _6, _10, _14]          [_8 , _9 , _10, _11]
	 * [_3, _7, _11, _15]          [_12, _13, _14, _15]
	 *
	 * orientation vectors (must be orthogonal, namely mutually perpendicular to each other)
	 * x (right)   [_0, _1, _2 ]
	 * y (up)      [_4, _5, _6 ]
	 * z (forward) [_8, _9, _10]
	 *
	 * origin
	 * position [_12, _13, _14]
	 *
	 * the bottom row (on the left) or right column (on the right)
	 * [_3, _7, _11, _15] is always [0, 0, 0, 1]
	 *
	 */
public:
	mat3<T> basis;  //< Storage for the rotation and the scale
	vec3<T> origin; //< Storage for the translation

public:
	explicit mat3x4(const mat3<T>& M = mat3<T>(T(1)), const vec3<T>& origin = vec3<T>(T(0))):
			basis(M),
			origin(origin)
	{
//		assert(M.det() >= std::numeric_limits<T>::epsilon());  // "input basis is a singular matrix"
	}

	explicit mat3x4(const vec3<T>& vx, const vec3<T>& vy, const vec3<T>& vz, const vec3<T>& origin = vec3<T>(T(0))):
			basis(vx, vy, vz),
			origin(origin)
	{
		assert(!isZero<T>(basis.det()));  // "three vector bases are coplanar"
	}

	vec3<T> getScale() const
	{
		// fast return
		if(basis.isDiagonal())
			return basis.diagonal();

		// We have to do the full calculation.
		vec3<T> v0(basis[0]), v1(basis[1]), v2(basis[2]);
		return vec3<T>(v0.length(), v1.length(), v2.length());
	}

	vec3<T> getRotation(const vec3<T>& scale) const
	{
		// scaling negatively on two axes is the equivalent of rotating 180 degrees
		vec3<T> s(scale);
		if(s.x < 0)
		{
			if(s.y < 0)
			{
				s.x = -s.x;
				s.y = -s.y;
			}
			else if(s.z < 0)
			{
				s.x = -s.x;
				s.z = -s.z;
			}
		}
		else if(s.y < 0&& s.z < 0)
		{
			s.y = -s.y;
			s.z = -s.z;
		}

		scale(1/s.x, 1/s.y, 1/s.z);
		return scale;
	}

//	static transform<T> multiply(const transform<T>& lhs, const transform<T>& rhs);
	friend mat3x4<T> operator *(const mat3x4<T>& lhs, const mat3x4<T>& rhs)
	{
		mat3x4<T> result(lhs);
		result *= rhs;
		return result;
	}

#ifdef ROW_COLUMN
	friend vec3<T> operator *(const vec3<T>& position, const transform<T>& xform)
	{
		return position * xform.basis + xform.origin;
	}
#else
	friend vec3<T> operator *(const mat3x4<T>& xform, const vec3<T>& position)
	{
		return xform.basis * position + xform.origin;
	}
#endif

	mat3x4<T>& operator *=(const mat3x4<T>& another)
	{
#ifdef ROW_COLUMN
		origin += another.origin * basis;
#else
		origin += basis * another.origin;
#endif
		basis *= another.basis;
		return *this;
	}

	bool operator ==(const mat3x4<T>& rhs) const
	{
		return basis == rhs.basis && origin == rhs.origin;
	}

	bool operator !=(const mat3x4<T>& rhs) const
	{
		return !(*this == rhs);
	}

	friend bool fuzzyEqual(const mat3x4<T>& lhs, const mat3x4<T>& rhs) { return lhs == rhs; }

	void loadIdentity()
	{
		basis.identity();
		origin = vec3<T>(T(0));
	}

	mat3x4<T> inverse() const
	{
		mat3<T> inv = basis.transpose();
		return mat3x4<T>(inv, inv * -origin);
	}

	/**
	 * It's equivalent to left multiplying by matrix
	 * [1     0           0      0]
	 * [0 cos(angle) -sin(angle) 0]
	 * [0 sin(angle)  cos(angle) 0]
	 * [0     0           0      1]
	 */
	mat3x4<T>& rotateX(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		T* a = basis.a;
		T tmp= s*a[1] + c*a[2];
		a[1] = c*a[1] - s*a[2];
		a[2] = tmp;

		tmp  = s*a[4] + c*a[5];
		a[4] = c*a[4] - s*a[5];
		a[5] = tmp;

		tmp  = s*a[7] + c*a[8];
		a[7] = c*a[7] - s*a[8];
		a[8] = tmp;

		T _z     = s*origin.y + c*origin.z;
		origin.y = c*origin.y - s*origin.z;
		origin.z = _z;

		return *this;
	}

	/**
	 * It's equivalent to left multiplying by matrix
	 * [ cos(angle) 0 sin(angle) 0]
	 * [    0       1     0      0]
	 * [-sin(angle) 0 cos(angle) 0]
	 * [    0       0     0      1]
	 */
	mat3x4<T>& rotateY(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		T* a = basis.a;

		T tmp= c*a[2] - s*a[0];
		a[0] = s*a[2] + c*a[0];
		a[2] = tmp;

		tmp  = c*a[5] - s*a[3];
		a[3] = s*a[5] + c*a[3];
		a[5] = tmp;

		tmp  = c*a[8] - s*a[6];
		a[6] = s*a[8] + c*a[6];
		a[8] = tmp;

		tmp = s*origin.z + c*origin.x;
		origin.z = c*origin.z - s*origin.x;
		origin.x = tmp;

		return *this;
	}

	/**
	 * It's equivalent to left multiplying by matrix
	 * [cos(angle) -sin(angle) 0  0]
	 * [sin(angle)  cos(angle) 0  0]
	 * [    0           0      1  0]
	 * [    0           0      0  1]
	 */
	mat3x4<T>& rotateZ(T angle)
	{
		const T c = std::cos(angle), s = std::sin(angle);
		T* a = basis.a;

		T tmp= s*a[0] + c*a[1];
		a[0] = c*a[0] - s*a[1];
		a[1] = tmp;

		tmp  = s*a[3] + c*a[4];
		a[3] = c*a[3] - s*a[4];
		a[4] = tmp;

		tmp  = s*a[6] + c*a[7];
		a[6] = c*a[6] - s*a[7];
		a[7] = tmp;

		tmp      = s*origin.x + c*origin.y;
		origin.x = c*origin.x - s*origin.y;
		origin.y = tmp;

		return *this;
	}

	/**
	 * rotate around vector counter-clockwise by angle
	 * note that the vector {@code unit} must be normalized before rotation.
	 *
	 * see {@link http://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle }
	 * It's equivalent to left multiplying by matrix
	 * [c+x*x*(1-c)   x*y*(1-c)-z*s x*z*(1-c)+y*s  0]
	 * [y*x*(1-c)+z*s c+y*y*(1-c)   y*z*(1-c)-x*s  0]
	 * [z*x*(1-c)-y*s z*y*(1-c)+x*s c+z*z*(1-c)    0]
	 * [      0             0             0        1]
	 * where x,y,z is normalized vector component,
	 * c denotes cos(angle), s denotes sin(angle).
	 */
	mat3x4<T>& rotate(const vec3<T>& unit, T angle)
	{
		assert(fuzzyEqual(T(1), unit.length()));  // v must be a normalized vector
		const vec3<T>&v = unit;
#if 0
		const vec3<T> p = v.polar();
		T phi = p.phi, theta = -p.theta + M_PI/2;
		(*this)
		.translate(-origin)  // translate to orgin point(0, 0, 0)
		.rotateZ(-phi)       // (rho, theta, phi) => (rho, theta, 0)
		.rotateY(-theta)     // (rho, theta, 0)   => (rho, pi/2, 0)
		.rotateX(angle)      // rotate around X(rho, pi/2, 0) axis
		.rotateY(theta)      // Y axis rotate back
		.rotateZ(phi)        // Z axis rotate back
		.translate(origin);  // translate back
#else
		const T c = std::cos(angle), C = 1-c, s = std::sin(angle);
		const T& x = v.x, &y = v.y, &z = v.z;
		mat3<T> M(
			   c + x*x*C, -z*s + x*y*C,  y*s + x*z*C,
			 z*s + y*x*C,    c + y*y*C, -x*s + y*z*C,
			-y*s + z*x*C,  x*s + z*y*C,    c + z*z*C);
#if COLUMN_MAJOR
		origin = M * origin;
		basis  = M * basis;
#else
		origin = origin * M;
		basis  = basis * M;
#endif
#endif
		return *this;
	}

	/**
	 * It's equivalent to left multiplying matrix
	 * [1 0 0 x]
	 * [0 1 0 y]
	 * [0 0 1 z]
	 * [0 0 0 1]
	 */
	mat3x4<T>& translate(const vec3<T>& v)
	{
		origin += v;
		return *this;
	}

	/**
	 * It's equivalent to left multiplying matrix
	 * [x 0 0 0]
	 * [0 y 0 0]
	 * [0 0 z 0]
	 * [0 0 0 1]
	 */
	mat3x4<T>& scale(const vec3<T>& v)
	{
		T* a = basis.a;
		a[0] *= v.x; a[3] *= v.x; a[6] *= v.x;
		a[1] *= v.y; a[4] *= v.y; a[7] *= v.y;
		a[2] *= v.z; a[5] *= v.z; a[8] *= v.z;
		return *this;
	}

	template <typename charT, class traits>
	friend std::basic_ostream<charT, traits>& operator <<(
			std::basic_ostream<charT, traits>& os, const mat3x4<T>& xform)
	{
		std::basic_ostringstream<charT, traits> stream;
		stream.flags(os.flags());
		stream.imbue(os.getloc());
		stream.precision(os.precision());
		stream << "basis:" << '\n'
				<< xform.basis
				<< "origin:" << '\n'
				<< xform.origin;
		return os << stream.str();
	}
};

using mat3x4f = mat3x4<float> ;
using mat3x4d = mat3x4<double>;

}  // namespace pea
#endif  // PEA_MATH_MAT3X4_H_
